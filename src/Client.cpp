#include "Client.h"


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
