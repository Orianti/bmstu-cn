#ifndef SERVER_H
#define SERVER_H

#include <unistd.h>
#include <arpa/inet.h>
#include <csignal>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <queue>

#include "../port.h"

#define THREAD_POOL_SIZE 10
#define MAX_CLIENTS_COUNT 100

struct client
{
    int socket;
    std::string ip;
};

const std::map<int, std::string> STATUSES{
        std::pair<int, std::string>{200, "OK"},
        std::pair<int, std::string>{403, "Forbidden"},
        std::pair<int, std::string>{404, "Not Found"},
        std::pair<int, std::string>{405, "Method Not Allowed"},
};

const std::map<std::string, std::string> TYPES{
        std::pair<std::string, std::string>{".html", "text/html"},
        std::pair<std::string, std::string>{".css", "text/css"},
};

#endif //SERVER_H
