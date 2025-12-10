#pragma once
#include <windows.h>
#include <string>
using namespace std;
const string MUTEX_NAME = "Lab4_FileMutex";
const string SEM_FREE_NAME = "Lab4_SemFree";
const string SEM_USED_NAME = "Lab4_SemUsed";
const string SEM_READY_NAME = "Lab4_SemReady";

const int MSG_SIZE = 20;

struct QueueHeader {
    int head;
    int tail;
    int count;
    int max_size;
};

inline int getOffset(int index) {
    return sizeof(QueueHeader) + (index * MSG_SIZE);
}