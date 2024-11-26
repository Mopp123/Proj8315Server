#pragma once

#include <stdexcept>
#include <string>
#include <mutex>
#include <utility>
#include <unordered_map>
#include <pqxx/pqxx>
#include "Debug.h"

// #define QUERY_ERR__NO_QUERY         -1
// #define QUERY_ERR__NO_ERROR         0
// #define QUERY_ERR__CONNECTION_ERROR 1
// #define QUERY_ERR__SYNTAX_ERROR     2
// #define QUERY_ERR__UNIQUE_VIOLATION 3
// #define QUERY_ERR__UNEXPECTED       4

#define QUERY_STATUS__INCOMPLETE        -1
#define QUERY_STATUS__SUCCESS           0
#define QUERY_STATUS__CONNECTION_ERROR  1
#define QUERY_STATUS__SYNTAX_ERROR      2
#define QUERY_STATUS__UNIQUE_VIOLATION  3
#define QUERY_STATUS__UNEXPECTED_ERROR  4

#define DATABASE_COLUMN__SERVER_INFO__MESSAGE 1

#define DATABASE_COLUMN__USERS__ID          0
#define DATABASE_COLUMN__USERS__NAME        1
#define DATABASE_COLUMN__USERS__LOGGED_IN   3
#define DATABASE_COLUMN__USERS__IS_ADMIN    4
#define DATABASE_COLUMN__USERS__TILE_X      5
#define DATABASE_COLUMN__USERS__TILE_Z      6


#define DATABASE_COLUMN__OBJECTS__ID          0
#define DATABASE_COLUMN__OBJECTS__TYPE_ID     1
#define DATABASE_COLUMN__OBJECTS__NAME        2
#define DATABASE_COLUMN__OBJECTS__DESCRIPTION 3
#define DATABASE_COLUMN__OBJECTS__STATS_SPEED 4
#define DATABASE_COLUMN__OBJECTS__ACTION_SLOT 5


#define DATABASE_COLUMN__FACTIONS__ID           0
#define DATABASE_COLUMN__FACTIONS__USER_ID      1
#define DATABASE_COLUMN__FACTIONS__NAME         2
#define DATABASE_COLUMN__FACTIONS__DEPLOYMENTS  3


class QueryResult
{
public:
    int status = QUERY_STATUS__INCOMPLETE;
    pqxx::result result;
    std::string errorMsg = "";

    template <typename T>
    T getValue(int row, int column)
    {
        try
        {
            return result[row][column].as<T>();
        }
        catch (const pqxx::argument_error& e)
        {
            Debug::log(
                "@QueryResult::getValue() Invalid argument: " + std::string(e.what()),
                Debug::MessageType::ERROR
            );
            return T();
        }
        catch (const pqxx::conversion_error& e)
        {
            Debug::log(
                "@QueryResult::getValue() Conversion error: " + std::string(e.what()),
                Debug::MessageType::ERROR
            );
            return T();
        }
        catch (const pqxx::range_error& e)
        {
            Debug::log(
                "@QueryResult::getValue() Range error: " + std::string(e.what()),
                Debug::MessageType::ERROR
            );
            return T();
        }
        catch (const std::exception& e)
        {
            Debug::log(
                "@QueryResult::getValue() Unexpected error: " + std::string(e.what()),
                Debug::MessageType::ERROR
            );
            return T();
        }
    }

    // Returns list of result's rows containing list of the specified column's array elements
    // [result rows][specified col's arr elements]
    template <typename T>
    std::vector<std::vector<T>> getArray(int column)
    {
        std::vector<std::vector<T>> ret;
        try
        {
            for (int i = 0; i < result.size(); ++i)
            {
                auto arr = result[i][column].as_array();
                std::pair<pqxx::array_parser::juncture, std::string> elem;
                std::vector<T> cells;
                do
                {
                    elem = arr.get_next();
                    if (elem.first == pqxx::array_parser::juncture::string_value)
                        cells.push_back(elem.second);
                }
                while (elem.first != pqxx::array_parser::juncture::done);
                ret.push_back(cells);
            }
        }
        catch (const pqxx::argument_error& e)
        {
            Debug::log(
                "@QueryResult::getValue() Invalid argument: " + std::string(e.what()),
                Debug::MessageType::ERROR
            );
        }
        catch (const pqxx::conversion_error& e)
        {
            Debug::log(
                "@QueryResult::getValue() Conversion error: " + std::string(e.what()),
                Debug::MessageType::ERROR
            );
        }
        catch (const pqxx::range_error& e)
        {
            Debug::log(
                "@QueryResult::getValue() Range error: " + std::string(e.what()),
                Debug::MessageType::ERROR
            );
        }
        catch (const std::exception& e)
        {
            Debug::log(
                "@QueryResult::getValue() Unexpected error: " + std::string(e.what()),
                Debug::MessageType::ERROR
            );
        }
        return ret;
    }
};


class DatabaseManager
{
private:
    static pqxx::connection* s_pConnection;
    static std::mutex s_mutex;

public:
    DatabaseManager(const DatabaseManager&) = delete;
    void operator=(DatabaseManager&) = delete;

    static bool connect(
        const std::string addr,
        const int port,
        const std::string dbName,
        const std::string dbUsername,
        const std::string dbPassword
    );

    static void disconnect();

    static QueryResult exec_query(const std::string sql);
};
