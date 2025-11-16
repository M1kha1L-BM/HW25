#include "Chat.h"
#include "Message.h"
#include "User.h"
#include <iostream>
#include <libpq-fe.h>

using namespace std;

Chat::Chat() {
    conn = PQconnectdb("host=localhost dbname=postgres user=postgres password=12345678");
    if (PQstatus(conn) != CONNECTION_OK) {
        cerr << "Ошибка подключения к БД: " << PQerrorMessage(conn) << endl;
        exit(1);
    }
    loadUsers();
    loadMessages();
}

Chat::~Chat() {
    PQfinish(conn);
    network.stop();
}

shared_ptr<User> Chat::findUserByLogin(const string& login) {
    for (auto& user : users)
        if (user->getLogin() == login)
            return user;
    return nullptr;
}

void Chat::loadUsers() {
    PGresult* res = PQexec(conn, "SELECT login, password, name FROM users");
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        cerr << "Ошибка SELECT users: " << PQerrorMessage(conn) << endl;
        PQclear(res);
        return;
    }
    int rows = PQntuples(res);
    for (int i = 0; i < rows; i++) {
        string login = PQgetvalue(res, i, 0);
        string password = PQgetvalue(res, i, 1);
        string name = PQgetvalue(res, i, 2);
        users.push_back(make_shared<User>(login, password, name));
    }
    PQclear(res);
}

void Chat::loadMessages() {
    const char* query =
        "SELECT m.sender, u.name, m.recipient, m.is_private, m.text "
        "FROM messages m "
        "JOIN users u ON m.sender = u.login "
        "ORDER BY m.id";

    PGresult* res = PQexec(conn, query);
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        cerr << "Ошибка SELECT messages: " << PQerrorMessage(conn) << endl;
        PQclear(res);
        return;
    }

    int rows = PQntuples(res);
    for (int i = 0; i < rows; i++) {
        string senderLogin = PQgetvalue(res, i, 0);
        string senderName = PQgetvalue(res, i, 1);
        string recipient = PQgetvalue(res, i, 2);
        bool isPrivate = (string(PQgetvalue(res, i, 3)) == "t");
        string text = PQgetvalue(res, i, 4);

        Message msg(senderLogin, recipient, text, isPrivate);
        msg.setSenderName(senderName);

        allMessages.push_back(msg);

        if (recipient != "all") {
            auto user = findUserByLogin(recipient);
            if (user) user->addMessage(msg);
        }
        else {
            for (auto& user : users)
                if (user->getLogin() != senderLogin)
                    user->addMessage(msg);
        }
    }
    PQclear(res);
}

void Chat::registerUser() {
    cout << "Регистрация нового пользователя" << endl;
    string login, password, name;

    cout << "Введите логин: ";
    cin >> login;
    if (findUserByLogin(login)) {
        cout << "Такой логин уже существует" << endl;
        return;
    }

    cout << "Введите пароль: ";
    cin >> password;
    cout << "Введите имя: ";
    cin.ignore();
    getline(cin, name);

    string query = "INSERT INTO users (login, password, name) VALUES ('" +
        login + "','" + password + "','" + name + "')";
    PGresult* res = PQexec(conn, query.c_str());
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        cerr << "Ошибка INSERT user: " << PQerrorMessage(conn) << endl;
    }
    PQclear(res);

    users.push_back(make_shared<User>(login, password, name));
    cout << "Пользователь зарегистрирован" << endl;
}

void Chat::login() {
    cout << "Вход в систему" << endl;
    string login, password;
    cout << "Логин: ";
    cin >> login;
    cout << "Пароль: ";
    cin >> password;

    auto user = findUserByLogin(login);
    if (user && user->getPassword() == password) {
        loggedInUser = user;
        cout << "Добро пожаловать, " << user->getName() << "!" << endl;
    }
    else {
        cout << "Неверный логин или пароль" << endl;
    }
}

void Chat::logout() {
    if (loggedInUser) {
        cout << "Вы вышли из системы" << endl;
        loggedInUser.reset();
    }
    else {
        cout << "Вы не вошли в систему" << endl;
    }
}

void Chat::sendMessage() {
    if (!loggedInUser) {
        cout << "Сначала войдите в систему" << endl;
        return;
    }

    cout << "Введите логин получателя (или 'all' для общего чата): ";
    string to;
    cin >> to;

    cout << "Введите сообщение: ";
    cin.ignore();
    string text;
    getline(cin, text);

    bool isPrivate = (to != "all");

    Message msg(loggedInUser->getLogin(), to, text, isPrivate);
    msg.setSenderName(loggedInUser->getName());
    allMessages.push_back(msg);

    string query = "INSERT INTO messages (sender, recipient, is_private, text) VALUES ('" +
        loggedInUser->getLogin() + "','" + to + "'," +
        (isPrivate ? "true" : "false") + ",'" + text + "')";
    PGresult* res = PQexec(conn, query.c_str());
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        cerr << "Ошибка INSERT message: " << PQerrorMessage(conn) << endl;
    }
    PQclear(res);

    if (to == "all") {
        for (auto& user : users)
            if (user->getLogin() != loggedInUser->getLogin())
                user->addMessage(msg);
        network.sendMessageToPeers(loggedInUser->getName() + " (всем): " + text);
        cout << "Сообщение отправлено всем пользователям" << endl;
    }
    else {
        auto recipient = findUserByLogin(to);
        if (recipient) {
            recipient->addMessage(msg);
            network.sendMessageToPeers(loggedInUser->getName() + " -> " + to + ": " + text);
            cout << "Сообщение отправлено пользователю " << recipient->getName() << "." << endl;
        }
        else {
            cout << "Пользователь не найден" << endl;
        }
    }
}

void Chat::checkMessages() {
    if (!loggedInUser) {
        cout << "Сначала войдите в систему" << endl;
        return;
    }
    loggedInUser->showInbox();
}

void Chat::startNetworking() {
    unsigned short port;
    cout << "Введите порт для вашего сервера (например, 54000): ";
    cin >> port;

    network.startServer(port);

    string ip;
    cout << "Введите IP другого узла (или 0, если нет): ";
    cin >> ip;

    if (ip != "0") {
        unsigned short peerPort;
        cout << "Введите порт другого узла: ";
        cin >> peerPort;
        network.connectToPeer(ip, peerPort);
    }
}

void Chat::run() {
    startNetworking();

    while (true) {
        cout << "\n--- Меню ---\n";
        cout << "1. Регистрация\n";
        cout << "2. Вход\n";
        cout << "3. Отправить сообщение\n";
        cout << "4. Проверить сообщения\n";
        cout << "5. Выйти из аккаунта\n";
        cout << "6. Выход из программы\n";
        cout << "Выберите действие: ";

        string choice;
        cin >> choice;

        if (choice == "1") registerUser();
        else if (choice == "2") login();
        else if (choice == "3") sendMessage();
        else if (choice == "4") checkMessages();
        else if (choice == "5") logout();
        else if (choice == "6") {
            cout << "Сохранение данных и выход..." << endl;
            break;
        }
        else cout << "Неверный ввод" << endl;
    }
}
