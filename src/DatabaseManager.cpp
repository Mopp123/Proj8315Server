#include "DatabaseManager.h"
#include "Debug.h"
#include <exception>
#include <mutex>


pqxx::connection* DatabaseManager::s_pConnection = nullptr;
std::mutex DatabaseManager::s_mutex;

bool DatabaseManager::connect(
    const std::string addr,
    const int port,
    const std::string dbName,
    const std::string dbUsername,
    const std::string dbPassword
)
{
    if (s_pConnection != nullptr)
    {
        Debug::log(
            "Attempted to connect database but connection was already established",
            Debug::MessageType::ERROR
        );
        return false;
    }
    else
    {
        std::string connArgs = "dbname = " + dbName + " user = " + dbUsername + " password = " + dbPassword + " hostaddr = " + addr + " port = " + std::to_string(port);
        try
        {
            s_pConnection = new pqxx::connection(connArgs);
            if (s_pConnection->is_open()) {
                Debug::log("Database connection established successfully");
                return true;
            } else {
                Debug::log(
                    "Failed to connect to database using: " + connArgs,
                    Debug::MessageType::ERROR
                );
                delete s_pConnection;
                return false;
            }
        }
        catch (const std::exception& e)
        {
            Debug::log(
                "Failed to connect to database using: " + connArgs + " Encountered exception: " + e.what(),
                Debug::MessageType::ERROR
            );
            if (s_pConnection != nullptr)
                delete s_pConnection;
            return false;
        }
    }
}

void DatabaseManager::disconnect()
{
    if (!s_pConnection)
    {
        Debug::log(
            "Attempted to close database connection but connection object was nullptr",
            Debug::MessageType::ERROR
        );
    }
    else
    {
        s_pConnection->close();
        delete s_pConnection;
    }
}

QueryResult DatabaseManager::exec_query(const std::string sql)
{
    if (!s_pConnection)
    {
        Debug::log(
            "Attempted to exec query but no connection to database exists",
            Debug::MessageType::FATAL_ERROR
        );
        return { QUERY_STATUS__CONNECTION_ERROR, pqxx::result() };
    }
    else
    {
        try
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            pqxx::work transaction(*s_pConnection);
            pqxx::result result = transaction.exec(sql);
            transaction.commit();
            return { QUERY_STATUS__SUCCESS, result };
        }
        catch (const pqxx::syntax_error& e)
        {
            return { QUERY_STATUS__SYNTAX_ERROR, pqxx::result(), e.what() };
        }
        catch (const pqxx::unique_violation& e)
        {
            return { QUERY_STATUS__UNIQUE_VIOLATION, pqxx::result(), e.what()};
        }
        catch (const std::exception& e)
        {
            return { QUERY_STATUS__UNEXPECTED_ERROR, pqxx::result(), e.what()};
        }
    }
}
