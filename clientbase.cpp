#include "clientbase.hpp"
#include <iostream>

ClientBase::ClientBase(std::string username, std::string ip_, std::uint16_t port) {
    ClientNode first_client = {std::move(username), std::move(ip_), port};
    std::string key = first_client.ip_ + std::to_string(first_client.port_);
    usersbase[key] = first_client;
}

ClientNode ClientBase::CreateClient(std::string username, std::string ip_, std::uint16_t port, int socket) {
    return {std::move(username), std::move(ip_), port, socket};
}

int ClientBase::AddClient(std::string username_, std::string ip_, std::uint16_t port_, int socket_) {
    ClientNode client = CreateClient(username_, ip_, port_, socket_);
    std::string key = client.ip_ + std::to_string(client.port_);
    usersbase[key] = client;
    return 1;
}

int ClientBase::AddClient(ClientNode client) {
    std::string key = client.ip_ + std::to_string(client.port_);
    usersbase[key] = client;
    return 1;
}

int ClientBase::RemoveClient(std::string ip_, std::uint16_t port_) {
    std::string key = ip_ + std::to_string(port_);
    usersbase.erase(key);
    return 1;
}

int ClientBase::RemoveClient(int socket) {
    for (auto it = usersbase.begin(); it != usersbase.end();) {
        if (it->second.socket_to_send_ == socket) {
            it = usersbase.erase(it);
        } else {
            ++it;
        }
    }
    return 1;
}

std::string ClientBase::MakePackage(const int socket) {
    std::string message = "01010101001 ";
    for (const auto& [key, client] : usersbase) {
        if (client.socket_to_send_ == -1 || client.socket_to_send_ == socket) {
            continue;
        }
        message += client.ip_ + " " + std::to_string(client.port_) + " " + client.username_ + " ";
    }
    return message;
}

void ClientBase::RenameClient(const std::string key, const std::string name) {
    if (usersbase.find(key) != usersbase.end()) {
        usersbase[key].username_ = name;
    }
}

void ClientBase::PrintClients() {
    for (const auto& [key, client] : usersbase) {
        std::cout << "IP: " << client.ip_ << ", Port: " << client.port_
                  << ", Username: " << client.username_ << "\n";
    }
}
