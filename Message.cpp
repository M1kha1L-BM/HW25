#include "Message.h"

Message::Message(const std::string& login,
    const std::string& recipient,
    const std::string& text,
    bool isPrivate)
    : senderLogin(login), recipient(recipient), text(text), isPrivate(isPrivate) {
}

std::string Message::getSenderLogin() const { return senderLogin; }
std::string Message::getSenderName() const { return senderName; }
std::string Message::getRecipient() const { return recipient; }
std::string Message::getText() const { return text; }
bool Message::getIsPrivate() const { return isPrivate; }

void Message::setSenderName(const std::string& name) { senderName = name; }