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
#include <unordered_map>

class client : public ClientBase
{
public:
    client(std::string username_, std::string ip_, std::uint16_t port_, bool mode);

    void CreatListeneSocet();
    void ConnectClient();
    void AcceptClient();
    void ReceiveContent(int ClientSocet);

private:
    ClientNode* node_ = nullptr;
    ClientBase* base_ = nullptr;

    std::unordered_map<std::string, int> socet_map;
    int socet_listen;
    sockaddr_in socet_listen_struct{};
};

