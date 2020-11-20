#include <cstdio>
#include <unistd.h>
#include <netinet/in.h>
#include <iostream>

#include "../port.h"

void get_request(int sd, const std::string &file)
{
    char buff[MSG_SIZE];
    sprintf(buff, "GET /%s HTTP/1.1\n", file.c_str());
    if (write(sd, buff, sizeof(buff)) == -1) {
        throw std::ios_base::failure(nullptr);
    }

    std::cout << "Request:\n" << buff << std::endl;
}

void get_response(int sd)
{
    char buff[MSG_SIZE] = {0};

    if (read(sd, buff, sizeof(buff) - 1) == -1) {
        throw std::ios_base::failure(nullptr);
    }

    std::cout << "Response:\n" << buff << std::endl;
}

int main(int argc, char **argv)
{
    std::string file = "index.html";
    if (argc == 2) {
        file = argv[1];
    } else if (argc > 2) {
        std::cerr << "too many arguments" << std::endl;
        return EXIT_FAILURE;
    }

    int sd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sd < 0) {
        std::cerr << "socket error" << std::endl;
        return EXIT_FAILURE;
    }

    const sockaddr_in client = {
            .sin_family = AF_INET,
            .sin_port = htons(CLIENT_PORT),
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

    if (bind(sd, (const sockaddr *) &client, sizeof(client)) < 0) {
        close(sd);
        std::cerr << "bind error" << std::endl;
        return EXIT_FAILURE;
    }

    const sockaddr_in server = {
            .sin_family = AF_INET,
            .sin_port = htons(PORT),
            .sin_addr = {
                    .s_addr = INADDR_ANY
            }
    };

    if (connect(sd, (const struct sockaddr *) &server, sizeof(server)) != 0) {
        close(sd);
        std::cerr << "connect error" << std::endl;
        return EXIT_FAILURE;
    }

    try {
        get_request(sd, file);
        get_response(sd);
    } catch (std::ios_base::failure &e) {
        close(sd);
        std::cerr << "io error" << std::endl;
    }

    close(sd);

    return EXIT_SUCCESS;
}
