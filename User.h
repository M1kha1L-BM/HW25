#pragma once
#include <string>
#include <vector>
#include "Message.h"

class User {
private:
    std::string login;
    std::string password;
    std::string name;
    std::vector<Message> inbox;

public:
    User(const std::string& login,
        const std::string& password,
        const std::string& name);

    std::string getLogin() const;
    std::string getPassword() const;
    std::string getName() const;

    void addMessage(const Message& msg);
    void showInbox() const;
};

