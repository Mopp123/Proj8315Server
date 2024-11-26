#include "Server.h"
#include "DatabaseManager.h"

#include <iostream>
#include <string>


int main(const int argc, const char** argv)
{
    // NOTE: Only temporarely connecting db here atm
    //  -> This is because Server's constrcutor creates "Game" and game's constructor needs db connection.
    // TODO: Figure out better way of handling this!
    if (!DatabaseManager::connect("127.0.0.1", 5432, "test_db", "postgres", "asd"))
    {
        Debug::log(
            "Failed to create server due to database connection error",
            Debug::MessageType::FATAL_ERROR
        );
        return 1;
    }
    Debug::log(
        "Database connection established successfully"
    );

    Server server("data/server-conf.txt", 1024);

    server.beginMsgHandler();
    server.beginGame();
    while(!server.is_shutting_down())
    {
        server.run();
    }
    server.shutdown();
    return 0;
}
