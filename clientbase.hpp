#pragma once

#include <vector>
#include <string>

struct ClientNode{
    std::string username_;
    std::string ip_;
    std::string port_;
};

class ClientBase
{
public:

    ClientBase() = default;

    ClientBase(std::string username, std::string ip_, std::string port);

    ClientNode CreateClient(std::string username, std::string ip_, std::string port);

    int AddClient(std::string username, std::string ip_, std::string port);

    int RemoveClient(std::string ip_, std::string port);

    void test();

private:

    std::vector<ClientNode> usersbase;
};


