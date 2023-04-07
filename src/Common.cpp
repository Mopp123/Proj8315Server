#include "Common.h"
#include "Debug.h"


User::User(const PK_byte* nameData, size_t nameSize, const PK_byte*  passwdData, size_t passwdSize)
{
    if (nameSize > USER_NAME_SIZE)
        nameSize = USER_NAME_SIZE;
    if (passwdSize > USER_PASSWD_SIZE)
        passwdSize = USER_PASSWD_SIZE;
    memset(_nameData, 0, USER_NAME_SIZE);
    memset(_passwordData, 0, USER_PASSWD_SIZE);
    if (nameData != nullptr)
        memcpy(_nameData, nameData, nameSize);
    if (passwdData != nullptr)
        memcpy(_passwordData, passwdData, passwdSize);
    _nameStr = std::string(nameData, nameSize);
}

User::User(const User& other) :
    _nameStr(other._nameStr),
    _isLoggedIn(other._isLoggedIn),
    _xPos(other._xPos),
    _zPos(other._zPos),
    _observeRadius(other._observeRadius),
    _factionName(other._factionName)
{
    memset(_nameData, 0, USER_NAME_SIZE);
    memset(_passwordData, 0, USER_PASSWD_SIZE);
    memcpy(_nameData, other._nameData, USER_NAME_SIZE);
    memcpy(_passwordData, other._passwordData, USER_PASSWD_SIZE);
}

void User::updateObserveProperties(int x, int z, int radius)
{
    _xPos = x;
    _zPos = z;
    _observeRadius = radius;
}

bool User::operator==(const User& other) const
{
    return memcmp(_nameData, other._nameData, USER_NAME_SIZE) == 0 &&
        _isLoggedIn == other._isLoggedIn &&
        _factionName == other._factionName;
}

bool User::operator!=(const User& other) const
{
    return memcmp(_nameData, other._nameData, USER_NAME_SIZE) != 0 ||
        _isLoggedIn != other._isLoggedIn ||
        _factionName != other._factionName;
}


Client::Client(std::string address, int connectionSD) :
    _address(address),
    _connSD(connectionSD)
{}

Client::Client(const Client& other):
    _address(other._address),
    _connSD(other._connSD)
{}

bool Client::operator==(const Client& other) const
{
    return _address == other._address && _connSD == other._connSD;
}

bool Client::operator!=(const Client& other) const
{
    return _address != other._address || _connSD != other._connSD;
}
