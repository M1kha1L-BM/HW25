#pragma once
#include <memory>
#include <vector>
#include <string>
#include "User.h"
#include "Message.h"
#include "Network.h"


#include <libpq-fe.h>

class Chat {
private:
    std::vector<std::shared_ptr<User>> users;
    std::vector<Message> allMessages;
    std::shared_ptr<User> loggedInUser = nullptr;
    Network network;

    // Соединение с PostgreSQL
    PGconn* conn;

    std::shared_ptr<User> findUserByLogin(const std::string& login);

    void loadUsers();
    void saveUsers();
    void loadMessages();
    void saveMessages();

public:
    Chat();
    ~Chat();

    void registerUser();
    void login();
    void logout();
    void sendMessage();
    void checkMessages();
    void startNetworking();
    void run();
};