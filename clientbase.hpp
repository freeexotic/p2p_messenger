#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <unordered_map>

struct ClientNode{
    std::string username_;
    std::string ip_;
    std::uint16_t port_;
    int socket_to_send_ = -1;
};

class ClientBase
{
public:

    ClientBase() = default;

    ClientBase(std::string username, std::string ip_, std::uint16_t port);
    // Создает вектор с описанием пользователей и добавлеяет информацию о первом пользователе

    ClientNode CreateClient(std::string username, std::string ip_, std::uint16_t port);
    // Создает node с информацией о пользователе

    int AddClient(std::string username, std::string ip_, std::uint16_t port);
    // Создает и добавляет клиента в вектор клиентов

    int AddClient(ClientNode client);
    // Добавляет уже сощданную ноду в вектор

    int RemoveClient(std::string ip_, std::uint16_t port);
    // Удаляет клиента из ClietBase

    void test();

private:

    std::unordered_map<std::string, ClientNode> usersbase;
};


