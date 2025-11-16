#pragma once
#include <string>
#include <thread>
#include <atomic>
#include <vector>
#include <mutex>
#include <iostream>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
typedef int SOCKET;
#endif

class Network {
public:
    Network();
    ~Network();

    bool startServer(unsigned short port);
    bool connectToPeer(const std::string& ip, unsigned short port);
    void sendMessageToPeers(const std::string& msg);
    void stop();

private:
    std::vector<SOCKET> peerSockets;
    std::atomic<bool> running;
    std::thread serverThread;
    std::mutex peersMutex;

    void serverLoop(unsigned short port);
    void handleClient(SOCKET clientSocket);
    void cleanupSocket(SOCKET sock);
};

