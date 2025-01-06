#include <iostream>
#include <clientbase.hpp>
#include <sstream>
#include <string>

int main()
{
    std::cout << "[1] - создать чат" << std::endl << "[2] - присоединиться к чату" << std::endl;
    int answer;
    std::cin >> answer;
    bool mode;
    switch(answer){
    case 1:
        std::cout << "Ответ 1" << std::endl;
        mode = true;
        break;
    case 2:
        std::cout << "Ответ 2" << std::endl;
        mode = false;
        break;
    default:
        std::cout << "Значение некорректно - перезапустите приложение!";
        break;
    }
}
