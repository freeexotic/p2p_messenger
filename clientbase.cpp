#include "clientbase.hpp"
#include <algorithm>
#include <iostream>


ClientBase::ClientBase(std::string username, std::string ip, std::uint16_t port) {
    ClientNode* first_client = new ClientNode;
    first_client->username_ = std::move(username);
    first_client->ip_ = std::move(ip);
    first_client->port_ = std::move(port);
    usersbase.push_back(std::move(*first_client));
}

ClientNode ClientBase::CreateClient(std::string username, std::string ip_, std::uint16_t port){
    ClientNode* new_client = new ClientNode;
    new_client->username_ = std::move(username);
    new_client->ip_ = std::move(ip_);
    new_client->port_ = std::move(port);
    return *new_client;
}

int ClientBase::AddClient(std::string username_, std::string ip_, std::uint16_t port_) {
    ClientNode client = CreateClient(username_, ip_, port_);
    usersbase.push_back(std::move(client));
    return 1;
}

int ClientBase::RemoveClient(std::string ip, std::uint16_t port){
    std::vector<ClientNode>::iterator iter = std::remove_if(usersbase.begin(), usersbase.end(),
        [&](ClientNode client){
        return client.ip_ == ip && client.port_ == port;
    });
    usersbase.erase(iter, usersbase.end());
    return 1;
}

void ClientBase::test(){
    for (auto i : usersbase){
        std::cout << i.username_ << std::endl;
    }
}
