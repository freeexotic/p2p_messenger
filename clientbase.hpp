#pragma once

#include <string>
#include <cstdint>
#include <unordered_map>

struct ClientNode {
    std::string username_;
    std::string ip_;
    std::uint16_t port_;
    int socket_to_send_ = -2;
};


class ClientBase
{
public:

    ClientBase() = default;

    ClientBase(std::string username, std::string ip_, std::uint16_t port);
    // Создает вектор с описанием пользователей и добавлеяет информацию о первом пользователе

    ClientNode CreateClient(std::string username, std::string ip_, std::uint16_t port, int socket = -2);
    // Создает node с информацией о пользователе

    int AddClient(std::string username, std::string ip_, std::uint16_t port, int socket = -2);
    // Создает и добавляет клиента в вектор клиентов

    int AddClient(ClientNode client);
    // Добавляет уже сощданную ноду в вектор

    int RemoveClient(std::string ip_, std::uint16_t port);
    // Удаляет клиента из ClietBase
    int RemoveClient(int socket);

    // Создает пакет с информацие о всех клиентах
    std::string MakePackage(const int socket);

    // Переименовать
    void RenameClient(const std::string key, const std::string name);

    void test();

private:

    std::unordered_map<std::string, ClientNode> usersbase;
};



class ClientBase {
public:
    ClientBase() = default;
    ClientBase(std::string username, std::string ip_, std::uint16_t port);

    ClientNode CreateClient(std::string username, std::string ip_, std::uint16_t port, int socket = -2);
    int AddClient(std::string username, std::string ip_, std::uint16_t port, int socket = -2);
    int AddClient(ClientNode client);
    int RemoveClient(std::string ip_, std::uint16_t port);
    int RemoveClient(int socket);
    std::string MakePackage(const int socket);
    void RenameClient(const std::string key, const std::string name);
    void PrintClients(); // Для отладки

private:
    std::unordered_map<std::string, ClientNode> usersbase;
};
