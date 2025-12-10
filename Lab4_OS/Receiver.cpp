#include <iostream>
#include <fstream>
#include <vector>
#include <windows.h>
#include "Common.h"

using namespace std;

int main() {
    setlocale(LC_ALL, "Russian");
    cout << "Процесс RECEIVER запушен." << endl;

    string filename;
    int queueSize;
    cout << "Введите имя бинарного файла: ";
    cin >> filename;
    cout << "Введите количество записей: ";
    cin >> queueSize;

    fstream file(filename, ios::out | ios::binary | ios::trunc);
    if (!file.is_open()) {
        cerr << "Ошибка создания файла!" << endl;
        return 1;
    }

    QueueHeader header = { 0, 0, 0, queueSize };
    file.write((char*)&header, sizeof(QueueHeader));

    char emptyBuffer[MSG_SIZE] = { 0 };
    for (int i = 0; i < queueSize; ++i) {
        file.write(emptyBuffer, MSG_SIZE);
    }
    file.close();
    cout << "Файл создан и инициализирован." << endl;

    int senderCount;
    cout << "Введите количество процессов Sender: ";
    cin >> senderCount;

    HANDLE hMutex = CreateMutex(NULL, FALSE, MUTEX_NAME.c_str());
    HANDLE hSemFree = CreateSemaphore(NULL, queueSize, queueSize, SEM_FREE_NAME.c_str());
    HANDLE hSemUsed = CreateSemaphore(NULL, 0, queueSize, SEM_USED_NAME.c_str());
    HANDLE hSemReady = CreateSemaphore(NULL, 0, senderCount, SEM_READY_NAME.c_str());

    if (!hMutex || !hSemFree || !hSemUsed || !hSemReady) {
        cerr << "Ошибка создания объектов синхронизации." << endl;
        return 1;
    }

    vector<STARTUPINFO> si(senderCount);
    vector<PROCESS_INFORMATION> pi(senderCount);

    for (int i = 0; i < senderCount; ++i) {
        ZeroMemory(&si[i], sizeof(STARTUPINFO));
        si[i].cb = sizeof(STARTUPINFO);
        ZeroMemory(&pi[i], sizeof(PROCESS_INFORMATION));

        string cmdArgs = "Lab4_OS_Sender.exe " + filename;

        vector<char> cmdBuffer(cmdArgs.begin(), cmdArgs.end());
        cmdBuffer.push_back(0);

        if (!CreateProcess(
            NULL,
            cmdBuffer.data(),
            NULL, NULL, FALSE,
            CREATE_NEW_CONSOLE,
            NULL, NULL,
            &si[i], &pi[i]))
        {
            cerr << "Ошибка запуска Sender " << i + 1 << endl;
        }
    }

    cout << "Жду готовности всех процессов Sender" << endl;
    for (int i = 0; i < senderCount; ++i) {
        WaitForSingleObject(hSemReady, INFINITE);
    }
    cout << "Все Sender-ы готовы к работе!" << endl;

    while (true) {
        cout << "\nВыберите действие:\n1. Читать сообщение\n2. Выход\n> ";
        int choice;
        cin >> choice;

        if (choice == 2) break;

        if (choice == 1) {
            cout << "Жду сообщения." << endl;

            WaitForSingleObject(hSemUsed, INFINITE);
            WaitForSingleObject(hMutex, INFINITE);

            file.open(filename, ios::in | ios::out | ios::binary);
            if (file.is_open()) {
                file.read((char*)&header, sizeof(QueueHeader));

                file.seekg(getOffset(header.head), ios::beg);
                char msgBuffer[MSG_SIZE];
                file.read(msgBuffer, MSG_SIZE);

                cout << "Receiver: Прочитано сообщение: " << msgBuffer << endl;

                header.head = (header.head + 1) % header.max_size;
                header.count--;

                file.seekp(0, ios::beg);
                file.write((char*)&header, sizeof(QueueHeader));
                file.close();
            }

            ReleaseMutex(hMutex);
            ReleaseSemaphore(hSemFree, 1, NULL);
        }
    }

    for (int i = 0; i < senderCount; ++i) {
        CloseHandle(pi[i].hProcess);
        CloseHandle(pi[i].hThread);
    }
    CloseHandle(hMutex);
    CloseHandle(hSemFree);
    CloseHandle(hSemUsed);
    CloseHandle(hSemReady);

    cout << "Работа завершена." << endl;
    return 0;
}