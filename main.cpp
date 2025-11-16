#include "Chat.h"
#include <iostream>

void printSystemInfo();

int main() {

    SetConsoleOutputCP(65001); // UTF-8
    setlocale(LC_ALL, "Russian");

    printSystemInfo();

    Chat chat;
    chat.run();

    return 0;
}