#include "clientbase.hpp"
#include <iostream>


ClientBase::ClientBase(std::string username, std::string ip, std::uint16_t port) {
    ClientNode* first_client = new ClientNode;
    std::string key = ip + std::to_string(port);
    first_client->username_ = std::move(username);
    first_client->ip_ = std::move(ip);
    first_client->port_ = std::move(port);
    usersbase[std::move(key)] = std::move(*first_client);
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
    std::string key = ip_ + std::to_string(port_);
    usersbase[std::move(key)] = std::move(client);
    return 1;
}

int ClientBase::AddClient(ClientNode client_node){
    std::string key = client_node.ip_ + std::to_string(client_node.port_);
    usersbase[std::move(key)] = std::move(client_node);
    return 1;
}

int ClientBase::RemoveClient(std::string ip, std::uint16_t port){
    std::string key = ip + std::to_string(port);
    usersbase.erase(key);
    return 1;
}

int ClientBase::RemoveClient(int socket){
    for (auto client : usersbase){
        if (client.second.socket_to_send_ == socket){
            std::string key = client.second.ip_ + std::to_string(client.second.port_);
            usersbase.erase(std::move(key));
            break;
        }
    }
    return 1;
}

void ClientBase::test(){
    for (auto key : usersbase){
        std::cout << key.second.username_ << std::endl;
    }
}

std::string ClientBase::MakePackage(){
    std::string message = "01010101001 ";
    for (auto client : usersbase){
        if(client.second.socket_to_send_ == -1){
            continue;
        }
        else{
            message += client.second.ip_;
            message += " ";
            message += std::to_string(client.second.port_);
            message += " ";
            message += client.second.username_;
            message += " ";
        }
    }
    return message;
}
