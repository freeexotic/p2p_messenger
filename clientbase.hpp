#pragma once

#include <vector>
#include <string>
#include <cstdint>

struct ClientNode{
    std::string username_;
    std::string ip_;
    std::uint16_t port_;
};

class ClientBase
{
public:

    ClientBase() = default;

    ClientBase(std::string username, std::string ip_, std::uint16_t port);

    ClientNode CreateClient(std::string username, std::string ip_, std::uint16_t port);

    int AddClient(std::string username, std::string ip_, std::uint16_t port);

    int RemoveClient(std::string ip_, std::uint16_t port);

    void test();

private:

    std::vector<ClientNode> usersbase;
};


