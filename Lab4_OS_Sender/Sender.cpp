#include <iostream>
#include <fstream>
#include <string>
#include <windows.h>
#include "Common.h"
#include <algorithm>
using namespace std;

int main(int argc, char* argv[]) {
    setlocale(LC_ALL, "Russian");
    cout << "Процесс SENDER запущен." << endl;

    if (argc < 2) {
        cerr << "Ошибка: Имя файла не передано через аргументы." << endl;
        system("pause");
        return 1;
    }

    string filename = argv[1];

    HANDLE hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, MUTEX_NAME.c_str());
    HANDLE hSemFree = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SEM_FREE_NAME.c_str());
    HANDLE hSemUsed = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SEM_USED_NAME.c_str());
    HANDLE hSemReady = OpenSemaphore(SEMAPHORE_ALL_ACCESS, FALSE, SEM_READY_NAME.c_str());

    if (!hMutex || !hSemFree || !hSemUsed || !hSemReady) {
        cerr << "Ошибка подключения к объектам синхронизации." << endl;
        system("pause");
        return 1;
    }

    cout << "Отправляю сигнал готовности" << endl;
    ReleaseSemaphore(hSemReady, 1, NULL);

    while (true) {
        cout << "\nВыберите действие:\n1. Отправить сообщение\n2. Выход\n> ";
        int choice;
        cin >> choice;
        cin.ignore();

        if (choice == 2) break;

        if (choice == 1) {
            string msg;
            cout << "Введите сообщение (макс 20 символов): ";
            getline(cin, msg);


            if (msg.length() >= MSG_SIZE) {
                msg = msg.substr(0, MSG_SIZE - 1);
            }

            cout << "Жду свободного места." << endl;

            WaitForSingleObject(hSemFree, INFINITE);
            WaitForSingleObject(hMutex, INFINITE);

            fstream file(filename, ios::in | ios::out | ios::binary);
            if (file.is_open()) {
                QueueHeader header;
                file.read((char*)&header, sizeof(QueueHeader));

                file.seekp(getOffset(header.tail), ios::beg);
                char msgBuffer[MSG_SIZE] = { 0 };

               strcpy_s(msgBuffer, msg.c_str());
                
               //copy(msg.begin(), msg.begin() + min((int)msg.size(), MSG_SIZE - 1), msgBuffer);
                
                file.write(msgBuffer, MSG_SIZE);

                header.tail = (header.tail + 1) % header.max_size;
                header.count++;

                file.seekp(0, ios::beg);
                file.write((char*)&header, sizeof(QueueHeader));
                file.close();

                cout << "Сообщение отправлено." << endl;
            }
            else {
                cerr << "Ошибка открытия файла." << endl;
            }

            ReleaseMutex(hMutex);
            ReleaseSemaphore(hSemUsed, 1, NULL);
        }
    }

    CloseHandle(hMutex);
    CloseHandle(hSemFree);
    CloseHandle(hSemUsed);
    CloseHandle(hSemReady);

    return 0;
}