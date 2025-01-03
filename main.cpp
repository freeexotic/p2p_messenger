#include <iostream>
#include <clientbase.hpp>

using namespace std;

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
    // ClientBase Base("111","112","121");
    // Base.AddClient("222", "221", "212");
    // Base.AddClient("333", "332", "323");
    // Base.RemoveClient("221", "212");
    // Base.test();
}
