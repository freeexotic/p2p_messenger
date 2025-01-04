#include "client.hpp"
#include <string>
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
#include <fcntl.h>
#include <functional>

client::client(std::string username_, std::string ip_, std::uint16_t port_, bool mode){
    if (mode == true){
        *node_ = CreateClient(username_, ip_, port_);
        *base_ = ClientBase(username_, ip_, port_);
        // реализовать оповещение о подключении - взаимодействие с пользователями
    }
    else{
        *node_ = CreateClient(username_, ip_, port_);
        // реализовать подключение к другому клиенту(серверу)
    }
}

void client::CreatListeneSocet(){
    if (node_ == nullptr){
        std::cout << "Client port and ip are not specified";
        return;
    }

    socet_listen = socket(AF_INET, SOCK_STREAM, 0);
    if (socet_listen < 0) {
        perror("Socket creation failed in clien");
        return;
    }    // создание сокета
    // sockaddr_in socet_listen_struct{}; - создается структура, которая будет содержать ip port (в hpp файле)
    socet_listen_struct.sin_family = AF_INET; // параметр, что используется протокол IPv4
    socet_listen_struct.sin_port = htons(node_->port_); // задается конкретный IP
    inet_pton(AF_INET, node_->ip_.c_str(), &socet_listen_struct.sin_addr); // задается конкретный порт

    if(bind(socet_listen, (struct sockaddr*)&socet_listen_struct, sizeof(socet_listen_struct)) < 0){
        perror("Bind faild");
        close(socet_listen);
        exit(EXIT_FAILURE);
    }
    // разобрать вывод ошибки
    // натройка соекта
    if (listen(socet_listen, 5) < 0) {
        perror("Listen failed");
        close(socet_listen);
        exit(EXIT_FAILURE);
    }
    // Создается очередь максимальной длиной в 5 клиентов
    // сокер в режиме прослушивания

    int flags = fcntl(socet_listen, F_GETFL, 0);
    fcntl(socet_listen, F_SETFL, flags | O_NONBLOCK);
    //Перевод сокета в неблокирующий режим

    while(true){
        int accept_client_socet = accept(socet_listen, nullptr, nullptr);

        if (accept_client_socet < 0){
            if( errno == EAGAIN || errno == EWOULDBLOCK){
                std::cout << "No incoming connections. Retrying..." << std::endl;
                sleep(1);
                continue;
                // Сообщаем о то, что нет ни одного соединения на подключение, ожидает 1 секунду перед проверкой
            }
            else{
                perror("Accept error");
                break;
                // Произошла иная ошибка подключения
            }
        }

        std::thread(&client::ReceiveContent, this, std::ref(accept_client_socet)).detach();

    }
}

void client::ConnectClient(){
    std::cout << "Enter the ip and port to connect to the client in the format: 127.0.0.1 1234" << std::endl;
    std::string ip;
    std::uint16_t port;
    std::getline(std::cin, ip);
    std::cin >> port;
// Вводится порт и ip клиента, которому необходимо подключиться
    int new_client_socet = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in new_client_socet_struct{};
    new_client_socet_struct.sin_family = AF_INET;
    new_client_socet_struct.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &new_client_socet_struct.sin_addr);

    if( connect(new_client_socet, (struct sockaddr*)&new_client_socet_struct, sizeof(new_client_socet_struct) ) < 0 ) {
        perror("Connect to new client failed");
        close(new_client_socet);
        return;
    }
    // Сделать, чтобы если было нарушено подключение(например очередь слишком длинная) , то клиент пробовал бы подключиться через какое-то время
// подключение к другому клиенту и создание другого сокета по которому будет происходить взаимодействие с клиентом
    std::string key;
    key = ip + std::to_string(port);
    socet_map[key] = new_client_socet;
// добавление клиента в список сокетов, чтобы можно было к нему обращаться
}

void client::AcceptClient(){

}

void client::ReceiveContent(int client_socet){
    while(true){

    }
}
