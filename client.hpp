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
    // создается клиент в зависимости от mode, и либо подключается к дургому клиенту и принимает все данные о других клиентах,
    // либо сощдает СlientBase и начинает прослушивать сокет, отвечающий за подключение

    void CreatListeneSocket();
    // создает сокет прослушивания

    void ConnectClient();
    // подключается к клиенту
    // используется при подключении к чату, когда происходит последовательное подключение ко всем клиентам
    // или используется, когда подключается новый клиент


    void AcceptClient();
    // Обрабатывает подключение клиента

    void ReceiveContent(int ClientSocket);
    // Принимает данные с сокета

private:
    void SendInfo(int&);
    void GetInfo(const std::string&);
    void ClientsInfo(int&);
    void SendMessage(int, const std::string&);


private:
    ClientNode* node_ = nullptr;
    ClientBase* base_ = nullptr;

    std::unordered_map<std::string, int> socet_map;
    // содержит соотношение ip:port и сокет
    int socet_listen;
    // сокет, отвечающий за прослушивание
    sockaddr_in socet_listen_struct{};
    // содержит информацию о сокете прослушивания
};

