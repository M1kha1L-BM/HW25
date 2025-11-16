#include "User.h"
#include <iostream>

User::User(const std::string& login,
    const std::string& password,
    const std::string& name)
    : login(login), password(password), name(name) {
}

std::string User::getLogin() const { return login; }
std::string User::getPassword() const { return password; }
std::string User::getName() const { return name; }

void User::addMessage(const Message& msg) {
    inbox.push_back(msg);
}

void User::showInbox() const {
    std::cout << "\n--- Сообщения для " << name << " ---\n";
    for (const auto& msg : inbox) {
        std::cout << msg.getSenderName() << " -> " << msg.getRecipient()
            << ": " << msg.getText();
        if (msg.getIsPrivate())
            std::cout << " [личное]";
        std::cout << std::endl;
    }
}