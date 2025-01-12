#include "client.hpp"
#include <string>
#include <iostream>
#include <thread>
#include <cstring> // для обработки строк
#include <sys/socket.h>
#include <netinet/in.h> // Для структуры sockaddr_in и констант протоколов.
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <functional>
#include <sstream>




client::client(std::string username_, std::string ip_, std::uint16_t port_, bool mode){

    *node_ = CreateClient(username_, ip_, port_);
    node_->socket_to_send_ = -1;
    *base_ = ClientBase(username_, ip_, port_);
    // Создается ClientNode клиента и ClientBase отвечающий за хранение всех ClientNode
    CreatListeneSocket();
    // Создание сокета, отвечающего за прослушивание

    if (mode == true){ // Проверка, нужно подключиться к какому-то клинету или выполнение роли создания чата
        // реализовать подключение к другому клиенту(серверу)
    }
}



void client::CreatListeneSocket(){
    if (node_ == nullptr){
        std::cout << "Client port and ip are not specified";
        return;
    }

    socket_listen = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_listen < 0) {
        perror("Socket creation failed in clien");
        return;
    }    // создание сокета
    // sockaddr_in socket_listen_struct{}; - создается структура, которая будет содержать ip port (в hpp файле)
    socket_listen_struct.sin_family = AF_INET; // параметр, что используется протокол IPv4
    socket_listen_struct.sin_port = htons(node_->port_); // задается конкретный IP
    inet_pton(AF_INET, node_->ip_.c_str(), &socket_listen_struct.sin_addr); // задается конкретный порт

    if(bind(socket_listen, (struct sockaddr*)&socket_listen_struct, sizeof(socket_listen_struct)) < 0){
        perror("Bind faild");
        close(socket_listen);
        exit(EXIT_FAILURE);
    }
    // разобрать вывод ошибки !!!!!!!!!!!!!!!!!!!!!!!!!!

    // натройка соекта
    if (listen(socket_listen, 5) < 0) {
        perror("Listen failed");
        close(socket_listen);
        exit(EXIT_FAILURE);
    }
    // Создается очередь максимальной длиной в 5 клиентов
    // сокер в режиме прослушивания

    int flags = fcntl(socket_listen, F_GETFL, 0);
    fcntl(socket_listen, F_SETFL, flags | O_NONBLOCK);
    //Перевод сокета в неблокирующий режим

    while(true){
        int accept_client_socket = accept(socket_listen, nullptr, nullptr);

        if (accept_client_socket < 0){
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
        std::thread(&client::ReceiveContent, this, std::ref(accept_client_socket)).detach();

    }
}



void client::ConnectClient() {
    std::cout << "Enter the ip and port to connect to the client in the format: 127.0.0.1 1234" << std::endl;
    std::string ip;
    std::uint16_t port;
    std::getline(std::cin, ip);
    std::cin >> port;
// Вводится порт и ip клиента, которому необходимо подключиться
    int new_client_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in new_client_socket_struct{};
    new_client_socket_struct.sin_family = AF_INET;
    new_client_socket_struct.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &new_client_socket_struct.sin_addr);

    if( connect(new_client_socket, (struct sockaddr*)&new_client_socket_struct, sizeof(new_client_socket_struct) ) < 0 ) {
        perror("Connect to new client failed");
        close(new_client_socket);
        return;
    }
    // Сделать, чтобы если было нарушено подключение(например очередь слишком длинная) , то клиент пробовал бы подключиться через какое-то время
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!

// добавление клиента в ClientBase
    ClientNode new_client = CreateClient("noname", ip, port, new_client_socket);
    base_->AddClient(new_client);
    SendInfo(new_client.socket_to_send_);
// отправление информации о себе для других клиентов
}



void client::ConnectWithoutAgreement(const std::string ip, const std::uint16_t port, const std::string username){
    int new_client_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in new_client_socket_struct{};
    new_client_socket_struct.sin_family = AF_INET;
    new_client_socket_struct.sin_port = htons(port);
    inet_pton(AF_INET, ip.c_str(), &new_client_socket_struct.sin_addr);

    if( connect(new_client_socket, (struct sockaddr*)&new_client_socket_struct, sizeof(new_client_socket_struct) ) < 0 ) {
        perror("Connect to new client failed");
        close(new_client_socket);
        return;
    }
    ClientNode new_client = CreateClient(username, ip, port, new_client_socket);
    base_->AddClient(new_client);
    SendInfoWithoutAgreement(new_client_socket);
}



void client::SendInfo(const int& new_client_socket) const{
    std::string info = "01010101011 " + node_->ip_ + " " + std::to_string(node_->port_) + " " + node_->username_;
    SendMessage(new_client_socket, info);
}



void client::SendInfoWithoutAgreement(const int& socket) const{
    std::string info = "01010111011 " + node_->ip_ + " " + std::to_string(node_->port_) + " " + node_->username_;
    SendMessage(socket, info);
}



void client::AcceptClient(const int& socket, const std::string& message){
    std::istringstream str_s(message);
    std::string ip;
    std::string username;
    std::uint16_t port;
    str_s >> ip >> port >> username;
    ClientNode new_client = CreateClient(username, ip, port, socket);
    base_->AddClient(new_client);
}



void client::ReceiveContent(int client_socket){
    char buffer[1024];
    std::string buffer_s;
    std::string word;
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0); // получение данных от клиента

        if (bytes_received <= 0) {
            close(client_socket);
            base_->RemoveClient(client_socket);
            break;
        } // обработка ситуации, если клиент отключился или произошла ошибка и передалось 0 или меньше байтов информации

        buffer_s = buffer;
        if(strncmp(buffer, "01010101011", 11) == 0 ){
            std::string message(buffer + 12, bytes_received - 12);
            AcceptClient(client_socket, message);
            SendName(client_socket);
            ClientsInfo(client_socket);
            continue;
        }
        else if(strncmp(buffer, "01010101111", 11) == 0 ){
            std::string message(buffer + 12, bytes_received - 12);
            std::istringstream str_s(message);
            std::string ip;
            std::string username;
            std::string port;
            str_s >> ip >> port >> username;
            base_->RenameClient(ip+port, username);
        } // Подтверждение подключение и передача имени клиенту
        else if(strncmp(buffer, "01010101001", 11) == 0 ){
            std::string message(buffer + 12, bytes_received - 12);
            GetInfo(message);
            // Добавление клиентов в ClientBase
            ConnectFromBase();
            // Подключение всех клиентов из ClientBase
        }
        else if (strncmp(buffer, "01010111011", 11) == 0 ){
            std::string message(buffer + 12, bytes_received - 12);
            AcceptClient(client_socket, message);
        }

        std::cout << "Received: " << buffer << std::endl;
        }
}



void client::GetInfo(const std::string& message) {
    std::istringstream str_s(message);
    std::string ip;
    std::string username;
    std::uint16_t port;

    // Чтение данных из строки
    while (str_s >> ip >> port >> username) {
    // Создание клиента и добавление в базу
        try {
            ConnectWithoutAgreement(ip, port, username);
        }
        catch (const std::exception& e) {
            std::cerr << "Error adding client: " << e.what() << std::endl;
        }
    }

    // Проверка состояния потока после завершения обработки
    if (!str_s.eof()) {
        std::cerr << "Warning." << std::endl;
    }
    return;
}



void client::ClientsInfo(const int& client_socket) const {
    std::string message = base_->MakePackage(client_socket);
    SendMessage(client_socket, message);
    return;
}



void client::SendMessage(const int socket, const std::string& message) const{
    int attempts = 0;
    while(attempts < 30){
        // Отправляем сообщение с указанием корректной длины строки
        ssize_t bytes_sent = send(socket, message.c_str(), message.size(), MSG_NOSIGNAL);

        if (bytes_sent == -1) {
            perror("Failed to send message to client");
            attempts++;
            continue;
        }
        else{
            return;
        }
        close(socket);
        perror("Error connecting to the client, try again later");
    }
    return;
}



void client::SendName(const int socket) const {
    std::string message = "01010101111 " + node_->ip_ + " " + std::to_string(node_->port_) + " " + node_->username_;
    SendMessage(socket, message);
}

