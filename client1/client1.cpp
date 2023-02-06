#define _WINSOCK_DEPRECATED_NO_WARNINGS
#pragma comment (lib, "ws2_32.lib")

// Пример простого TCP-клиента
#include <stdio.h>
#include <string>
#include <winsock2.h>
#include <windows.h>
#include <locale>
#include <iostream>

#define PORT 10000
#define SERVERADDR "127.0.0.1"

using namespace std;

string recieve(SOCKET my_sock)
{
    char buff[5000];
    int nsize = 0;
    nsize = recv(my_sock, &buff[0], sizeof(buff), 0);
    if (nsize == -1)
        nsize = 0;
    buff[nsize] = 0;

    string answer(buff);

    cout << answer << endl;

    if (answer.length() < 2) {
        return answer;
    }

    return answer.substr(1, answer.length() - 2);
}

int inputDeck() {
    int iDeck;
    cout << ">> ";
    cin >> iDeck;

    while (cin.fail() || iDeck < 1 || iDeck > 4) {
        cout << "Ошибка. Введите число от 1 до 4" << endl;
        cin.clear();
        cin.ignore(256, '\n');
        cout << ">> ";
        cin >> iDeck;
    }
    return iDeck;
}

string inputCell() {
    
    char letter;
    int number;

    cout << "Клетка:\n>> ";
    cin >> letter >> number;

    while (cin.fail() || !('A' <= letter && letter <= 'K') || !(1 <= number && number <= 10)) {
        cout << "Ошибка формата." << endl;
        cin.clear();
        cin.ignore(256, '\n');
        cout << "Клетка\n>> ";
        cin >> letter >> number;
    }
    return letter + to_string(number);
}

void Prepare(SOCKET my_sock) {
    // Самостоятельная расстановка
    string answer = recieve(my_sock);

    char cmd[10];

    int iDeck = -1;

    while (true) {
        
        Sleep(100);

        // Карта
        answer = recieve(my_sock);

        Sleep(100);

        // Осталось разместить
        answer = recieve(my_sock);

        Sleep(100);
        
        // Вопрос про размер
        // Ввод размера
        iDeck = inputDeck();

        cmd[0] = to_string(iDeck)[0];
        cmd[1] = '\0';

        send(my_sock, &cmd[0], 2, 0);

        Sleep(100);

        // Введите клетки
        answer = recieve(my_sock);

        string cells = "";
        for (int i = 0; i < iDeck; i++) {
            cells += inputCell();
        }

        send(my_sock, cells.c_str(), cells.length() + 1, 0);

        Sleep(50);

        // Хорошо или Ошибка
        answer = recieve(my_sock);
        cout << "[" << answer << "]" << endl;
        Sleep(100);

        // Готовы или Продолжаем
        answer = recieve(my_sock);

        cout << "{" << answer << "}" << endl;

        if (answer == "Ваши корабли готовы!") {
            cout << "123" << endl;
            break;
        }

    }

    cout << "Ожидание расстановки кораблей соперником..." << endl;
}

void Battle(SOCKET my_sock) {
    
    cout << "Начался обмен ударами!" << endl;

    while (true)
    {
        // Ожидание или Ваш ход
        string answer = recieve(my_sock);

        if (answer == "Ожидание хода другого игрока!") {
            answer = recieve(my_sock);
            if (answer == "Вы проиграли") {
                system("pause");
                break;
            }
        }

        Sleep(100);

        // Карта
        answer = recieve(my_sock);

        Sleep(100);

        string cell = inputCell();

        // Послали клетку
        send(my_sock, cell.c_str(), cell.length() + 1, 0);

        Sleep(50);

        // Попадание или Мимо
        answer = recieve(my_sock);

        if (answer == "Попадание!") {
            Sleep(100);

            // Потоплен или Поврежден
            answer = recieve(my_sock);

            if (answer == "Корабль потоплен!") {
                Sleep(100);

                // Выиграли или Игра продолжается
                answer = recieve(my_sock);
                
                if (answer == "Вы выиграли") {
                    system("pause");
                    break;
                }
            }
        }

    }
}

int main()
{
    setlocale(LC_ALL, "RUS");

    WSAData wsaData;
    char buff[5000];
    printf("Клиент из стартед!\n");
    // Шаг 1 - инициализация библиотеки Winsock
    if (WSAStartup(MAKEWORD(2, 2), (WSADATA*)&wsaData))
    {
        printf("WSAStart ошибка %i\n", WSAGetLastError());
        return -1;
    }
    // Шаг 2 - создание сокета
    SOCKET my_sock;
    my_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (my_sock < 0)
    {
        printf("Socket() ошибка %i\n", WSAGetLastError());
        return -1;
    }
    // Шаг 3 - установка соединения
    // заполнение структуры sockaddr_in – указание адреса и порта сервера
    sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);
    HOSTENT* hst;
    // преобразование IP адреса из символьного в сетевой формат
    if (inet_addr(SERVERADDR) != INADDR_NONE)
        dest_addr.sin_addr.s_addr = inet_addr(SERVERADDR);
    else
        // попытка получить IP адрес по доменному имени сервера
        if (hst = gethostbyname(SERVERADDR))
            // hst->h_addr_list содержит не массив адресов,
            // а массив указателей на адреса
            ((unsigned long*)&dest_addr.sin_addr)[0] =
            ((unsigned long**)hst->h_addr_list)[0][0];
        else
        {
            printf("Инвалид адрес %s\n", SERVERADDR);
            closesocket(my_sock);
            WSACleanup();
            return -1;
        }
    // адрес сервера получен – пытаемся установить соединение
    if (connect(my_sock, (sockaddr*)&dest_addr, sizeof(dest_addr)))
    {
        printf("Коннект эррор %i\n", WSAGetLastError());
        return -1;
    }
    printf("Соединение с %s успешно установлено\n\
         Наберите quit для завершения\n\n", SERVERADDR);
    // Шаг 4 - чтение и передача сообщений

    // Выберите 1 или 2
    string answer = recieve(my_sock);

    char cmd[10];

    printf(">> ");
    fflush(stdout);

    fgets(&cmd[0], sizeof(cmd) - 1, stdin);
    cmd[1] = '\0';

    send(my_sock, &cmd[0], 2, 0);

    // Из файла или Самостоятельная
    answer = recieve(my_sock);

    if (answer == "Самостоятельная расстановка!") {
        Prepare(my_sock);
    }

    Battle(my_sock);

    printf("Recv error %d\n", WSAGetLastError());
    closesocket(my_sock);
    WSACleanup();
    return -1;
}