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
#include <chrono>




client::client(std::string username_, std::string ip_, std::uint16_t port_, int mode) {
    node_ = CreateClient(username_, ip_, port_);
    node_.socket_to_send_ = -1;
    base_ = ClientBase(username_, ip_, port_);

    // Отладочный вывод значения mode
    std::cout << "Mode внутри конструктора: " << mode << std::endl;

    try {
        if (mode == 1) {
            std::cout << "Создание сокета для чата" << std::endl;
            // Запуск потока прослушивания
            listen_thread = std::thread(&client::CreatListeneSocket, this);
            listen_thread.detach();
        } else if (mode == 2) {
            std::cout << "Создание сокета для чата и для прослушивания" << std::endl;
            // Запуск потока прослушивания и подключения
            listen_thread = std::thread(&client::CreatListeneSocket, this);
            listen_thread.detach();
            // Небольшая задержка перед подключением клиента
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::thread connect_thread(&client::ConnectClient, this);
            connect_thread.detach();
        } else {
            throw std::invalid_argument("Некорректное значение mode");
        }
    } catch (const std::exception& e) {
        std::cerr << "Ошибка в конструкторе client: " << e.what() << std::endl;
    }
}

client::~client()
{
    run_listen_loop = false;
    if (listen_thread.joinable()) {
        listen_thread.join();
    }
    if (socket_listen != -1) {
        close(socket_listen);
    }
}


void client::CreatListeneSocket(){
    std::cout << "Ожидание подключения ч0";
    socket_listen = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_listen < 0) {
        perror("Socket creation failed in clien");
        return;
    }    // создание сокета
    std::cout << "Ожидание подключения ч1";
    // sockaddr_in socket_listen_struct{}; - создается структура, которая будет содержать ip port (в hpp файле)
    socket_listen_struct.sin_family = AF_INET; // параметр, что используется протокол IPv4
    socket_listen_struct.sin_port = htons(node_.port_); // задается конкретный порт
    inet_pton(AF_INET, node_.ip_.c_str(), &socket_listen_struct.sin_addr); // задается конкретный ip

    if(bind(socket_listen, (struct sockaddr*)&socket_listen_struct, sizeof(socket_listen_struct)) < 0){
        perror("Bind faild");
        close(socket_listen);
        socket_listen = -1;
        exit(EXIT_FAILURE);
    }
    // разобрать вывод ошибки !!!!!!!!!!!!!!!!!!!!!!!!!!
    std::cout << "Ожидание подключения ч2";
    // натройка соекта
    if (listen(socket_listen, 5) < 0) {
        perror("Listen failed");
        close(socket_listen);
        socket_listen = -1;
        exit(EXIT_FAILURE);
    }
    // Создается очередь максимальной длиной в 5 клиентов
    // сокер в режиме прослушивания

    int flags = fcntl(socket_listen, F_GETFL, 0);
    fcntl(socket_listen, F_SETFL, flags | O_NONBLOCK);
    //Перевод сокета в неблокирующий режим

    std::cout << "Ожидание подключения3";
    while(true){
        int accept_client_socket = accept(socket_listen, nullptr, nullptr);

        if (accept_client_socket < 0){
            if( errno == EAGAIN || errno == EWOULDBLOCK){
               // std::cout << "No incoming connections. Retrying..." << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(1));
                continue;
                // Сообщаем о то, что нет ни одного соединения на подключение, ожидает 1 секунду перед проверкой
            }
            else{
                perror("Accept error");
                break;
                // Произошла иная ошибка подключения
            }
        }
        std::cout << "Подключислся новый пользователь!!!!";
        std::thread(&client::ReceiveContent, this, std::ref(accept_client_socket)).detach();

    }
}



void client::ConnectClient() {
    std::cout << "Введите ip и порт в формате: 127.0.0.1 1234" << std::endl;
    std::string ip;
    std::uint16_t port;
    std::cin >> ip >> port;
// Вводится порт и ip клиента, которому необходимо подключиться
    std::cout << "Подключение произошло";
    int new_client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (new_client_socket < 0) {
        perror("Socket creation failed in ConnectClient");
        return;
    }
    std::cout << "Подключение произошло";
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
    base_.AddClient(new_client);

 // отправление информации о себе для других клиентов
    SendInfo(new_client.socket_to_send_);
}



void client::ConnectWithoutAgreement(const std::string ip, const std::uint16_t port, const std::string username){
    int new_client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (new_client_socket < 0) {
        perror("Socket creation failed in ConnectWithoutAgreement");
        return;
    }

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

    base_.AddClient(new_client);
    SendInfoWithoutAgreement(new_client_socket);
}



void client::SendInfo(const int& new_client_socket) const{
    std::string info = "01010101011 " + node_.ip_ + " " + std::to_string(node_.port_) + " " + node_.username_;
    SendMessage(new_client_socket, info);
}



void client::SendInfoWithoutAgreement(const int& socket) const{
    std::string info = "01010111011 " + node_.ip_ + " " + std::to_string(node_.port_) + " " + node_.username_;
    SendMessage(socket, info);
}



void client::AcceptClient(const int& socket, const std::string& message){
    std::istringstream str_s(message);
    std::string ip;
    std::string username;
    std::uint16_t port;
    str_s >> ip >> port >> username;

    ClientNode new_client = CreateClient(username, ip, port, socket);
    base_.AddClient(new_client);
}



void client::ReceiveContent(int client_socket){
    char buffer[1024];
    while (true) {
        memset(buffer, 0, sizeof(buffer));
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0); // получение данных от клиента

        if (bytes_received <= 0) {
            close(client_socket);
            base_.RemoveClient(client_socket);
            break;
        } // обработка ситуации, если клиент отключился или произошла ошибка и передалось 0 или меньше байтов информации

        if(strncmp(buffer, "01010101011", 11) == 0 ){
            std::string message(buffer + 12, bytes_received - 12);
            AcceptClient(client_socket, message);
            SendName(client_socket);
            ClientsInfo(client_socket);
            std::cout << "send name";
            continue;
        }
        else if(strncmp(buffer, "01010101111", 11) == 0 ){
            std::string message(buffer + 12, bytes_received - 12);
            std::istringstream str_s(message);
            std::string ip;
            std::string username;
            std::string port;
            str_s >> ip >> port >> username;
            base_.RenameClient(ip + port, username);
            // Подтверждение подключение и передача имени клиенту
        }
        else if(strncmp(buffer, "01010101001", 11) == 0 ){
            std::string message(buffer + 12, bytes_received - 12);
            GetInfo(message);
            // Добавление клиентов в ClientBase
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



void client::ClientsInfo(const int& client_socket) {
    std::string message = base_.MakePackage(client_socket);
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
}



void client::SendName(const int socket) const {
    std::string message = "01010101111 " + node_.ip_ + " " + std::to_string(node_.port_) + " " + node_.username_;
    SendMessage(socket, message);
}

