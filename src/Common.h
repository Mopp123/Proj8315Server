#pragma once

#include <cstring>
#include <string>

#define PK_byte char
#define PK_ubyte unsigned char

#define DIR_N 	0
#define DIR_NE 	1
#define DIR_E 	2
#define DIR_SE 	3
#define DIR_S 	4
#define DIR_SW 	5
#define DIR_W 	6
#define DIR_NW 	7

#define USER_ID_LEN 32

#define CLIENT_NAME_LEN 32
#define USER_NAME_SIZE 	32
#define USER_PASSWD_SIZE 32
#define MOTD_LEN        256

#define NULL_CLIENT Client("", 0)
#define NULL_USER User(nullptr, 0, nullptr, 0)


class Server;


class User
{
private:
    PK_byte _nameData[USER_NAME_SIZE];
    PK_byte _passwordData[USER_PASSWD_SIZE];
    std::string _nameStr = "";
    bool _isLoggedIn = false;

    int _xPos = 0;
    int _zPos = 0;
    int _observeRadius = 15;

    // Key used to identify the user's faction in "Game"
    // NOTE: Used std::string here, since this data is not supposed to be netw data
    std::string _factionName = "";

public:
    User() {}
    User(const PK_byte* nameData, size_t nameSize, const PK_byte*  passwdData, size_t passwdSize);
    User(const User& other);

    void updateObserveProperties(int x, int z, int radius);
    bool operator==(const User& other) const;
    bool operator!=(const User& other) const;

    inline const int& getX() const { return _xPos; }
    inline const int& getZ() const { return _zPos; }
    inline const int& getObserveRadius() const { return _observeRadius; }

    inline  bool isLoggedIn() const { return _isLoggedIn; }

    inline const PK_byte* getNameData() const { return _nameData; }
    inline const PK_byte* getPasswordData() const { return _passwordData; }
    inline const std::string& getName() const { return _nameStr; }
    inline const std::string& getFactionName() const { return _factionName; }
    inline void setFactionName(const std::string& name) { _factionName = name; }
};


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
