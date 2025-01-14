#include <iostream>
#include "client.hpp"
#include <sstream>
#include <string>

int main() {
    std::string ip;
    std::uint16_t port;
    std::string username;

    // Ввод данных пользователя
    std::cout << "Введите Ваш IP в формате: 127.0.0.1" << std::endl;
    std::cin >> ip;

    std::cout << "Введите Ваш port в формате: 1234" << std::endl;
    std::cin >> port;

    std::cout << "Введите Ваш username" << std::endl;
    std::cin >> username;

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

    // Основной цикл программы
    while (true) {
        // Логика программы - обработка пользовательских команд
    }

    return 0;
}
