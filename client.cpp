#include "client.hpp"
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <cstring> // для обработки строк
#include <sys/socket.h>
#include <netinet/in.h> // Для структуры sockaddr_in и констант протоколов.
#include <unistd.h>
#include <algorithm>
#include <arpa/inet.h>

client::client(std::string username_, std::string ip_, std::string port_, bool mode){
    if (mode == true){
        *node_ = CreateClient(username_, ip_, port_);
        *base_ = ClientBase(username_, ip_, port_);
    }
    else{
        *node_ = CreateClient(username_, ip_, port_);
    }
}

void client::CreateSocet(){
    if (node_ == nullptr){
        std::cout << "Client port and ip are not specified";
        return;
    }

    client_socet_listen = socket(AF_INET, SOCK_STREAM, 0); // создание сокета
    // sockaddr_in socet_struct{}; - создается структура, которая будет содержать ip port
    socet_struct.sin_family = AF_INET; // параметр, что используется протокол IPv4
    socet_struct.sin_port = htons(std::stoi(node_->port_)); // задается конкретный IP
    inet_pton(AF_INET, node_->ip_.c_str(), &socet_struct.sin_addr); // задается конкретный порт

    if(bind(client_socet_listen, (struct sockaddr*)&socet_struct, sizeof(socet_struct)) < 0){
        perror("Bind faild");
        close(client_socet_listen);
        exit(EXIT_FAILURE);
    }
    // натройка соекта
    if (listen(client_socet_listen, 5) < 0) {
        perror("Listen failed");
        close(client_socet_listen);
        exit(EXIT_FAILURE);
    }
    // сокер в режиме прослушивания

}
