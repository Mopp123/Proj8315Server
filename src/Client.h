#pragma once

#include <string>


#define CLIENT_NAME_LEN 32

#define NULL_CLIENT Client("", 0)


class Client
{
private:
    std::string _address = "";
    int _connSD = 0;

    friend class Server;

public:
    Client() {}
    Client(std::string address, int connectionSD);
    Client(const Client& other);

    bool operator==(const Client& other) const;
    bool operator!=(const Client& other) const;

    inline const std::string& getAddress() const { return _address; }
    inline int getConnSD() const { return _connSD; }
};
