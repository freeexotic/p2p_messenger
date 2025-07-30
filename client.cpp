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
#include <poll.h>




client::client(std::string username_, std::string ip_, std::uint16_t port_, int mode) :
    base_(username_, ip_, port_)
{
    node_ = CreateClient(username_, ip_, port_);
    node_.socket_to_send_ = -1;

    // Отладочный вывод значения mode
    std::cout << "Mode внутри конструктора: " << mode << std::endl;

    try {
        if (mode != 1 && mode != 2){
            std::cout << "Некорректное значение mode" << std::endl;
            return;
        }
        listen_thread = std::thread(&client::CreatListeneSocket, this, std::ref(mode));
        listen_thread.detach();
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


void client::CreatListeneSocket(int mode) {
    std::cout << "Ожидание подключения: начало" << std::endl;

    // Создание сокета
    socket_listen = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_listen < 0) {
        perror("Ошибка создания сокета");
        return;
    }

    // Настройка структуры сокета
    socket_listen_struct.sin_family = AF_INET; // Используется IPv4
    socket_listen_struct.sin_port = htons(node_.port_); // Установка порта
    inet_pton(AF_INET, node_.ip_.c_str(), &socket_listen_struct.sin_addr); // Установка IP-адреса

    // Привязка сокета
    if (bind(socket_listen, reinterpret_cast<struct sockaddr*>(&socket_listen_struct), sizeof(socket_listen_struct)) < 0) {
        perror("Ошибка привязки сокета");
        close(socket_listen);
        socket_listen = -1;
        return;
    }

    // Перевод сокета в режим прослушивания
    if (listen(socket_listen, 5) < 0) {
        perror("Ошибка установки режима прослушивания");
        close(socket_listen);
        socket_listen = -1;
        return;
    }

    // Перевод сокета в неблокирующий режим
    int flags = fcntl(socket_listen, F_GETFL, 0);
    if (flags == -1 || fcntl(socket_listen, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("Ошибка перевода сокета в неблокирующий режим");
        close(socket_listen);
        socket_listen = -1;
        return;
    }

    std::cout << "Сокет успешно настроен и ожидает подключения" << std::endl;

    // Если режим подключения клиента (mode == 2), вызываем ConnectClient()
    if (mode == 2) {
        ConnectClient();
    }

    run_listen_loop = true;

    // Основной цикл обработки подключений
    while (run_listen_loop) {
        int accept_client_socket = accept(socket_listen, nullptr, nullptr);

        if (accept_client_socket < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::this_thread::sleep_for(std::chrono::seconds(5)); // Задержка перед повторной проверкой
                continue;
            } else {
                perror("Ошибка при принятии подключения");
                break;
            }
        }

        std::cout << "Подключился новый пользователь!" << std::endl;

        // Обработка нового подключения в отдельном потоке
        std::thread(&client::ReceiveContent, this, std::ref(accept_client_socket)).detach();
    }

    close(socket_listen); // Закрытие сокета после завершения работы
}




void client::ConnectClient() {
    std::cout << "Введите IP и порт для подключения (например: 127.0.0.1 1234): ";

    std::string ip;
    uint16_t port;
    
    if (!(std::cin >> ip >> port)) {
        std::cerr << "Ошибка ввода IP и порта" << std::endl;
        std::cin.clear();
        std::cin.ignore(10000, '\n');
        return;
    }

    int new_client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (new_client_socket < 0) {
        perror("Ошибка создания сокета для подключения");
        return;
    }

    struct sockaddr_in new_client_socket_struct{};
    new_client_socket_struct.sin_family = AF_INET;
    new_client_socket_struct.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &new_client_socket_struct.sin_addr) <= 0) {
        perror("Некорректный IP-адрес");
        close(new_client_socket);
        return;
    }

    if (connect(new_client_socket, reinterpret_cast<struct sockaddr*>(&new_client_socket_struct), sizeof(new_client_socket_struct)) < 0) {
        perror("Ошибка подключения к клиенту");
        close(new_client_socket);
        return;
    }

    std::cout << "Успешное подключение к клиенту!" << std::endl;

    // Добавление клиента в базу данных
    ClientNode new_client = CreateClient("noname", ip, port, new_client_socket);
    base_.AddClient(new_client);

    // Отправка информации о себе новому клиенту
    SendInfo(new_client.socket_to_send_);
}



void client::ConnectWithoutAgreement(const std::string ip, const uint16_t port, const std::string username) {
    int new_client_socket = socket(AF_INET, SOCK_STREAM, 0);

    if (new_client_socket < 0) {
        perror("Ошибка создания сокета для подключения без согласования");
        return;
    }

    struct sockaddr_in new_client_socket_struct{};
    new_client_socket_struct.sin_family = AF_INET;
    new_client_socket_struct.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &new_client_socket_struct.sin_addr) <= 0) {
        perror("Некорректный IP-адрес");
        close(new_client_socket);
        return;
    }

    if (connect(new_client_socket, reinterpret_cast<struct sockaddr*>(&new_client_socket_struct), sizeof(new_client_socket_struct)) < 0) {
        perror("Ошибка подключения к клиенту без согласования");
        close(new_client_socket);
        return;
    }

    ClientNode new_client = CreateClient(username, ip, port, new_client_socket);

    base_.AddClient(new_client);

    SendInfoWithoutAgreement(new_client_socket);
}




void client::SendInfo(const int& new_client_socket) const{
    std::cout << "Информатция отправленна";
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



void client::ReceiveContent(int client_socket) {
    std::cout << "Клиент подключился, ожидание сообщений..." << std::endl;

    // Структура для опроса сокета
    struct pollfd pfd;
    pfd.fd = client_socket; // Указываем сокет
    pfd.events = POLLIN; // Будем проверять готовность на чтение

    char buffer[1024];

    while (true) {
        // Ожидаем данных в течение 1000 мс
        int poll_result = poll(&pfd, 1, 1000);
        if (poll_result < 0) {
            perror("poll error");
            break;
        } else if (poll_result == 0) {
            // Нет данных, переходим на новую итерацию
            continue;
        }

        // Если сокет готов на чтение, пробуем получить данные
        if (pfd.revents & POLLIN) {
            memset(buffer, 0, sizeof(buffer));
            int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);

            // Проверка на ошибки и отключение клиента
            if (bytes_received < 0) {
                // Для незначительных ошибок делаем паузу и пробуем снова
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                } else {
                    perror("Ошибка при получении данных от клиента");
                    break;
                }
            } else if (bytes_received == 0) {
                std::cout << "Клиент отключился." << std::endl;
                close(client_socket);
                base_.RemoveClient(client_socket);
                break;
            }

            // Обрабатываем полученное сообщение
            std::string received_message(buffer, bytes_received);

            if (strncmp(buffer, "01010101011", 11) == 0) {
                HandleAcceptClient(client_socket, buffer, bytes_received);
            } else if (strncmp(buffer, "01010101111", 11) == 0) {
                HandleRenameClient(client_socket, buffer, bytes_received);
            } else if (strncmp(buffer, "01010101001", 11) == 0) {
                HandleAddClients(buffer, bytes_received);
            } else if (strncmp(buffer, "01010111011", 11) == 0) {
                HandleSimpleAccept(client_socket, buffer, bytes_received);
            } else if (strncmp(buffer, "MESSAGE", 7) == 0) {
                std::string message(buffer + 8, bytes_received - 8);
                std::cout << "\n" << message << std::endl;
                std::cout << "Введите команду: " << std::flush;
            } else {
                std::cout << "Получено неизвестное сообщение: " << received_message << std::endl;
            }
        }
    }
}


void client::HandleAcceptClient(int client_socket, const char* buffer, int bytes_received) {
    std::cout << "Обработка команды: отправка имени" << std::endl;

    std::string message(buffer + 12, bytes_received - 12);
    AcceptClient(client_socket, message);
    SendName(client_socket);
    ClientsInfo(client_socket);
}

void client::HandleRenameClient(int client_socket, const char* buffer, int bytes_received) {
    std::cout << "Обработка команды: подтверждение и передача имени клиенту" << std::endl;

    std::string message(buffer + 12, bytes_received - 12);
    std::istringstream str_s(message);

    std::string ip;
    std::string username;
    uint16_t port;

    str_s >> ip >> port >> username;
    base_.RenameClient(ip + std::to_string(port), username);
}


void client::HandleAddClients(const char* buffer, int bytes_received) {
    std::cout << "Обработка команды: добавление клиентов" << std::endl;

    std::string message(buffer + 12, bytes_received - 12);
    GetInfo(message); // Добавление клиентов в ClientBase
}

void client::HandleSimpleAccept(int client_socket, const char* buffer, int bytes_received) {
    std::cout << "Обработка команды: простое подтверждение подключения" << std::endl;

    std::string message(buffer + 12, bytes_received - 12);
    AcceptClient(client_socket, message);
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
    if (socket < 0) {
        std::cerr << "Ошибка: недопустимый сокет" << std::endl;
        return;
    }
    
    int attempts = 0;
    const int max_attempts = 3;
    
    while(attempts < max_attempts){
        ssize_t bytes_sent = send(socket, message.c_str(), message.size(), MSG_NOSIGNAL);

        if (bytes_sent == -1) {
            if (errno == EPIPE || errno == ECONNRESET) {
                std::cerr << "Соединение разорвано для сокета " << socket << std::endl;
                break;
            } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                attempts++;
                continue;
            } else {
                perror("Ошибка отправки сообщения");
                attempts++;
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }
        } else if (bytes_sent == 0) {
            std::cerr << "Соединение закрыто удаленной стороной" << std::endl;
            break;
        } else {
            return;
        }
    }
    
    if (attempts >= max_attempts) {
        std::cerr << "Не удалось отправить сообщение после " << max_attempts << " попыток" << std::endl;
    }
}



void client::SendName(const int socket) const {
    std::string message = "01010101111 " + node_.ip_ + " " + std::to_string(node_.port_) + " " + node_.username_;
    SendMessage(socket, message);
}

void client::SendMessageToAll(const std::string& message) const {
    std::string formatted_message = "MESSAGE " + node_.username_ + ": " + message;
    for (const auto& [key, client_node] : base_.usersbase) {
        if (client_node.socket_to_send_ != -1 && client_node.socket_to_send_ != -2) {
            SendMessage(client_node.socket_to_send_, formatted_message);
        }
    }
}

void client::ShowConnectedClients() const {
    std::cout << "\n=== Подключенные клиенты ===" << std::endl;
    base_.PrintClients();
    std::cout << "========================\n" << std::endl;
}

void client::ProcessUserInput() {
    std::string input;
    std::cout << "\nДоступные команды:" << std::endl;
    std::cout << "/msg <сообщение> - отправить сообщение всем" << std::endl;
    std::cout << "/list - показать подключенных клиентов" << std::endl;
    std::cout << "/quit - выйти из программы" << std::endl;
    std::cout << "\nВведите команду или сообщение: ";
    
    while (std::getline(std::cin, input)) {
        if (input.empty()) {
            std::cout << "Введите команду: ";
            continue;
        }
        
        if (input == "/quit") {
            std::cout << "Выход из программы..." << std::endl;
            Shutdown();
            break;
        } else if (input == "/list") {
            ShowConnectedClients();
        } else if (input.substr(0, 4) == "/msg") {
            if (input.length() > 5) {
                std::string message = input.substr(5);
                SendMessageToAll(message);
                std::cout << "Сообщение отправлено." << std::endl;
            } else {
                std::cout << "Использование: /msg <сообщение>" << std::endl;
            }
        } else {
            SendMessageToAll(input);
            std::cout << "Сообщение отправлено." << std::endl;
        }
        
        std::cout << "Введите команду: ";
    }
}

void client::Shutdown() {
    std::cout << "Закрываем соединения..." << std::endl;
    
    // Закрываем все активные соединения
    for (const auto& [key, client_node] : base_.usersbase) {
        if (client_node.socket_to_send_ > 0) {
            close(client_node.socket_to_send_);
        }
    }
    
    // Останавливаем поток прослушивания
    run_listen_loop = false;
    
    // Закрываем сокет прослушивания
    if (socket_listen > 0) {
        close(socket_listen);
    }
    
    std::cout << "Завершение работы завершено." << std::endl;
}

