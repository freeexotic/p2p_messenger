#include <iostream>
#include "client.hpp"
#include <sstream>
#include <string>
#include <thread>
#include <chrono>
#include <regex>

bool isValidIP(const std::string& ip) {
    std::regex ipPattern(R"(^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$)");
    return std::regex_match(ip, ipPattern);
}

bool isValidPort(std::uint16_t port) {
    return port > 0 && port <= 65535;
}

bool isValidUsername(const std::string& username) {
    return !username.empty() && username.length() <= 50 && 
           username.find(' ') == std::string::npos;
}

int main() {
    std::string ip;
    std::uint16_t port;
    std::string username;

    // Ввод данных пользователя с валидацией
    do {
        std::cout << "Введите Ваш IP в формате: 127.0.0.1" << std::endl;
        std::cin >> ip;
        if (!isValidIP(ip)) {
            std::cout << "Некорректный IP адрес! Попробуйте снова." << std::endl;
        }
    } while (!isValidIP(ip));

    do {
        std::cout << "Введите Ваш port в формате: 1234" << std::endl;
        std::cin >> port;
        if (std::cin.fail()) {
            std::cin.clear();
            std::cin.ignore(10000, '\n');
            std::cout << "Некорректный формат порта! Попробуйте снова." << std::endl;
            continue;
        }
        if (!isValidPort(port)) {
            std::cout << "Port должен быть в диапазоне 1-65535! Попробуйте снова." << std::endl;
        }
    } while (!isValidPort(port));

    do {
        std::cout << "Введите Ваш username (без пробелов, до 50 символов)" << std::endl;
        std::cin >> username;
        if (!isValidUsername(username)) {
            std::cout << "Некорректное имя пользователя! Попробуйте снова." << std::endl;
        }
    } while (!isValidUsername(username));

    int mode;
    // Ввод режима работы
    std::cout << "[1] - создать чат\n[2] - присоединиться к чату" << std::endl;
    std::cin >> mode;

    // Проверка ввода
    if (std::cin.fail() || (mode != 1 && mode != 2)) {
        std::cerr << "Значение некорректно - перезапустите приложение!" << std::endl;
        return 1;
    }

    // Создание клиента
    client c(username, ip, port, mode);

    // Небольшая задержка для инициализации
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Основной цикл программы - обработка пользовательских команд
    c.ProcessUserInput();

    return 0;
}
