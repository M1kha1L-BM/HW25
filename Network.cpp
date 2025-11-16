#include "Network.h"
#include <cstring>

Network::Network() : running(false) {
#ifdef _WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

Network::~Network() {
    stop();
#ifdef _WIN32
    WSACleanup();
#endif
}

void Network::cleanupSocket(SOCKET sock) {
#ifdef _WIN32
    closesocket(sock);
#else
    close(sock);
#endif
}

bool Network::startServer(unsigned short port) {
    running = true;
    serverThread = std::thread(&Network::serverLoop, this, port);
    return true;
}

void Network::serverLoop(unsigned short port) {
    SOCKET serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock == INVALID_SOCKET) {
        std::cerr << "[Ошибка] Не удалось создать сокет сервера\n";
        return;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "[Ошибка] bind()\n";
        cleanupSocket(serverSock);
        return;
    }

    listen(serverSock, 5);
    std::cout << "[Сервер] Ожидание подключений на порту " << port << "...\n";

    while (running) {
        sockaddr_in clientAddr{};
#ifdef _WIN32
        int clientLen = sizeof(clientAddr);
#else
        socklen_t clientLen = sizeof(clientAddr);
#endif
        SOCKET clientSock = accept(serverSock, (sockaddr*)&clientAddr, &clientLen);
        if (clientSock != INVALID_SOCKET) {
            std::thread(&Network::handleClient, this, clientSock).detach();
        }
    }

    cleanupSocket(serverSock);
}

void Network::handleClient(SOCKET clientSocket) {
    char buffer[1024];
    while (running) {
        int bytes = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) break;
        buffer[bytes] = '\0';
        std::cout << "\n[Получено] " << buffer << "\n> ";
    }
    cleanupSocket(clientSocket);
}

bool Network::connectToPeer(const std::string& ip, unsigned short port) {
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) return false;

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
#ifdef _WIN32
    InetPtonA(AF_INET, ip.c_str(), &addr.sin_addr);
#else
    inet_pton(AF_INET, ip.c_str(), &addr.sin_addr);
#endif

    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        std::cerr << "[Ошибка] Не удалось подключиться к " << ip << ":" << port << "\n";
        cleanupSocket(sock);
        return false;
    }

    std::cout << "[Подключено] " << ip << ":" << port << "\n";
    std::lock_guard<std::mutex> lock(peersMutex);
    peerSockets.push_back(sock);
    return true;
}

void Network::sendMessageToPeers(const std::string& msg) {
    std::lock_guard<std::mutex> lock(peersMutex);
    for (auto sock : peerSockets) {
        send(sock, msg.c_str(), (int)msg.size(), 0);
    }
}

void Network::stop() {
    running = false;
    std::lock_guard<std::mutex> lock(peersMutex);
    for (auto sock : peerSockets)
        cleanupSocket(sock);
    peerSockets.clear();

    if (serverThread.joinable())
        serverThread.join();
}
