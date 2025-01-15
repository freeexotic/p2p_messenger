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
    client(std::string username_, std::string ip_, std::uint16_t port_, int mode);
    // создается клиент в зависимости от mode, и либо подключается к дургому клиенту и принимает все данные о других клиентах,
    // либо сощдает СlientBase и начинает прослушивать сокет, отвечающий за подключение
    ~client();

private:

    void CreatListeneSocket(int mode);
    // создает сокет прослушивания

    void ConnectClient();
    // подключается к клиенту
    // используется при подключении к чату, когда происходит последовательное подключение ко всем клиентам
    // или используется, когда подключается новый клиент

    void ConnectWithoutAgreement(const std::string ip, const std::uint16_t port, const std::string username);

    void ReceiveContent(int ClientSocket);
    // Принимает данные с сокета

    void AcceptClient(const int&,  const std::string&);
    // Обрабатывает подключение клиента

    void SendInfo(const int&) const;
    // Отправляет информацию содержащую свой port, ip, username

    void SendInfoWithoutAgreement(const int&) const;
    // Отправляет информацию содержащую свой port, ip, username без подтверждения подключения

    void GetInfo(const std::string&);
    // Обрабатывает получаемую информацию и добавляет новые ClientNode в ClientBase

    void ClientsInfo(const int&);
    // Создает пакет, содержащий информацию о всех клиентах и отпраляет его

    void SendMessage(const int, const std::string&) const;
    // Метод позволяющий отправлять данные всем клиентам

    void SendName(const int socket) const;
    // Отправляет имя и подтверждает подключение

private:

    void HandleAcceptClient(int client_socket, const char* buffer, int bytes_received);
    // Выполняет подключение клиента

    void HandleRenameClient(int client_socket, const char* buffer, int bytes_received);
    // Подтверждение подключения и приянятие имени

    void HandleAddClients(const char* buffer, int bytes_received);
    // Добавление клиента

    void HandleSimpleAccept(int client_socket, const char* buffer, int bytes_received);
    // Простое подключение клиента


private:
    ClientNode node_;
    // Структура, содержащая информацию о клиенте

    ClientBase base_;
    // Класс, хранящий ClientNode и позволяющий работать с ними

    // std::unordered_map<std::string, int> socet_map;
    // // содержит соотношение ip:port и сокет

    int socket_listen;
    // сокет, отвечающий за прослушивание

    sockaddr_in socket_listen_struct{};
    // содержит информацию о сокете прослушивания

    bool run_listen_loop = false;


    std::thread listen_thread;
};


