#include "server.h"

int sd = 0;

pthread_t thread_pool[THREAD_POOL_SIZE];

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

std::queue<client> client_queue;

std::array<long, 24> request_frequency;

void write_log()
{
    auto now = time(nullptr);

    pthread_mutex_lock(&log_mutex);
    ++request_frequency[localtime(&now)->tm_hour];
    pthread_mutex_unlock(&log_mutex);
}

void save_log(const std::string &filename)
{
    std::ofstream fd(filename);
    for (const auto &hour : request_frequency) {
        fd << hour << "\n";
    }
    fd.close();
}

std::string string_to_hex(const std::string &input)
{
    const char DIGITS[] = "0123456789ABCDEF";

    std::string output;
    output.reserve(input.length() * 2);
    for (auto c : input) {
        output.push_back(DIGITS[c >> 4]);
        output.push_back(DIGITS[c & 15]);
    }

    return output;
}

std::map<std::string, std::string> get_headers(const std::string &header_text)
{
    std::istringstream header_list{header_text};

    std::string line;
    std::map<std::string, std::string> headers;

    while (std::getline(header_list, line) && line != "\n") {
        auto pos = line.find(':');
        headers[line.substr(0, pos)] = line.substr(pos + 2, line.length() - pos - 1);
    }

    return headers;
}

std::string response_format(const std::string &protocol, int status, std::map<std::string, std::string> headers,
                            const std::string &body)
{
    auto response = protocol + " " + std::to_string(status) + " " + STATUSES.at(status);
    std::cout << response << std::endl;

    response += "\n";
    std::string tmp[2] = {"Content-Type", "Content-Length"};
    for (const auto &header : tmp) {
        if (headers.find(header) != headers.end()) {
            response += header + ": " + headers.at(header) + "\n";
        }
    }
    response += "\n" + body;

    return response;
}

std::tuple<std::string, std::string, std::string> split_request(const std::string &line)
{
    auto method_end = line.find(' ');
    auto path_end = line.find(' ', method_end + 1);

    auto method = line.substr(0, method_end);
    auto path = line.substr(method_end + 1, path_end - method_end - 1);
    auto protocol = line.substr(path_end + 1, line.length() - path_end - 1);
    if (path[0] == '/' && path.length() != 1) {
        path = path.substr(1, path.length() - 1);
    } else {
        path = "index.html";
    }

    return {method, path, protocol};
}

std::string get_content_type(const std::string &path)
{
    auto pos = path.find('.');
    auto ext = path.substr(pos, path.length() - pos);

    return TYPES.find(ext) == TYPES.end() ? "text/plain" : TYPES.at(ext);
}

std::string get_response(const client &client, const std::string &request)
{
    write_log();

    std::map<std::string, std::string> response_headers;
    std::string body;

    auto line_end = request.find('\n');
    auto line = request.substr(0, line_end);

    std::cout << "[" << client.ip << "] request: " << line << std::endl;
    std::cout << "[" << client.ip << "] response: ";

    auto headers = get_headers(request.substr(line_end + 1, request.length() - line_end));

    int status = 200;

    auto[method, path, protocol] = split_request(line);
    if (method != "GET" || protocol != "HTTP/1.1") {
        status = 405;
        return response_format(protocol, status, response_headers, body);
    }

    std::ifstream infile{path};
    if (!infile.good()) {
        status = 404;
        return response_format(protocol, status, response_headers, body);
    }
    body = std::string{std::istreambuf_iterator<char>(infile), std::istreambuf_iterator<char>()};
    infile.close();

    response_headers["Content-Type"] = get_content_type(path);
    if (response_headers["Content-Type"].find("image") != std::string::npos) {
        body = string_to_hex(body);
    }
    response_headers["Content-Length"] = std::to_string(body.length());

    return response_format(protocol, status, response_headers, body);
}

void sigint_handler([[maybe_unused]] int signum)
{
    close(sd);
    std::cout << "The server is stopped" << std::endl;

    std::string log_filename = "log.txt";
    save_log(log_filename);
    std::cout << "Log file " << log_filename << "is saved successfully" << std::endl;

    exit(EXIT_SUCCESS);
}

void process(const client &client)
{
    char buff[MSG_SIZE + 1] = {0};
    if (read(client.socket, buff, sizeof(buff)) == -1) {
        throw std::ios_base::failure(nullptr);
    }

    auto response = get_response(client, std::string(buff));

    if (write(client.socket, response.c_str(), response.length()) == -1) {
        throw std::ios_base::failure(nullptr);
    }
}

[[noreturn]] void *thread_routine([[maybe_unused]] void *argv)
{
    while (true) {
        client *client = nullptr;

        pthread_mutex_lock(&client_mutex);
        if (client_queue.empty()) {
            pthread_cond_wait(&cond, &client_mutex);
            client = &client_queue.front();
            client_queue.pop();
        }
        pthread_mutex_unlock(&client_mutex);

        if (client) {
            process(*client);
        }
    }
}

int main()
{
    for (auto &i : thread_pool) {
        pthread_create(&i, nullptr, thread_routine, nullptr);
    }

    sd = socket(AF_INET, SOCK_STREAM, 0);
    if (sd == -1) {
        std::cerr << "socket error" << std::endl;
        return EXIT_FAILURE;
    }

    const sockaddr_in server_sa = {
            .sin_family = AF_INET,
            .sin_port = htons(PORT),
            .sin_addr = {
                    .s_addr = INADDR_ANY
            }
    };

    size_t size = 1;
    if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &size, sizeof(int)) == -1) {
        close(sd);
        std::cerr << "setsockopt error" << std::endl;
        return EXIT_FAILURE;
    }

    if (bind(sd, (const sockaddr *) &server_sa, sizeof(server_sa)) == -1) {
        close(sd);
        std::cerr << "bind error" << std::endl;
        return EXIT_FAILURE;
    }

    if (listen(sd, MAX_CLIENTS_COUNT) == -1) {
        close(sd);
        std::cerr << "listen error" << std::endl;
        return EXIT_FAILURE;
    }

    sockaddr_in client_sa{};
    socklen_t client_sa_size = sizeof(client_sa);

    signal(SIGINT, sigint_handler);

    std::cout << "Server is running.." << std::endl;
    while (true) {
        int client_sd = accept(sd, (sockaddr * ) & client_sa, &client_sa_size);
        if (client_sd == -1) {
            close(sd);
            std::cerr << "accept error" << std::endl;
            return EXIT_FAILURE;
        }

        client client = {
                .socket = client_sd,
                .ip = std::string(inet_ntoa(client_sa.sin_addr))
                      + ":" + std::to_string(ntohs(client_sa.sin_port))
        };

        pthread_mutex_lock(&client_mutex);
        client_queue.push(client);
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&client_mutex);
    }
}
