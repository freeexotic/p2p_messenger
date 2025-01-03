#pragma once

#include <clientbase.hpp>
#include <thread>
#include <mutex>
#include <cstring> // для обработки строк
#include <sys/socket.h>
#include <netinet/in.h> // Для структуры sockaddr_in и констант протоколов.
#include <unistd.h>
#include <algorithm>
#include <arpa/inet.h>

class client : public ClientBase
{
public:
    client(std::string username_, std::string ip_, std::string port_, bool mode);

    void CreateSocet();

private:
    ClientNode* node_ = nullptr;
    ClientBase* base_ = nullptr;

    int client_socet_listen;
    sockaddr_in socet_struct{};
};

