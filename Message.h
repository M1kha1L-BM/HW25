#pragma once
#include <string>

class Message {
private:
    std::string senderLogin;   // логин отправителя (для базы)
    std::string senderName;    // имя отправителя (для отображения)
    std::string recipient;     // логин получателя или "all"
    std::string text;
    bool isPrivate;

public:
    Message(const std::string& login,
        const std::string& recipient,
        const std::string& text,
        bool isPrivate);

    // Геттеры
    std::string getSenderLogin() const;
    std::string getSenderName() const;
    std::string getRecipient() const;
    std::string getText() const;
    bool getIsPrivate() const;

    // Сеттер для имени (подтягивается через JOIN)
    void setSenderName(const std::string& name);
};