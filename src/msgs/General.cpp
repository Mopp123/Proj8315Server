#include "General.h"
#include "Server.h"
#include "DatabaseManager.h"
#include "../../Proj8315Common/src/messages/GeneralMessages.h"
#include "../../Proj8315Common/src/messages/WorldMessages.h"
#include "../../Proj8315Common/src/Faction.h"
#include "game/Game.h"
#include "General.h"
#include "game/objects/Object.h"
#include <string>
// NOTE: only temporarely using this here!!
#include <iostream> // -> use rather Debug

using namespace gamecommon;

namespace msgs
{
    // TODO: Create message of type "GetServerMessage" which takes just takes the message string as parameter
    Message get_server_message(Server& server, const Client& client, Message& msg)
    {
        std::string message = "Welcome! Testing testing...";

        size_t bufSize = MESSAGE_SIZE__ServerMessageResponse;
        GC_byte respData[bufSize];
        memset(respData, 0, bufSize);

        const int32_t messageType = MESSAGE_TYPE__ServerMessage;
        memcpy(respData, &messageType, sizeof(int32_t));
        memcpy(respData + sizeof(int32_t), message.data(), message.size());

        // NOTE: just a hack atm
        return Message(respData, bufSize, bufSize);
    }


    // TODO: Injection prevention
    Message user_login(Server& server, const Client& client, Message& msg)
    {
        LoginRequest loginReqMsg(msg.getData(), msg.getDataSize());
        if (loginReqMsg != NULL_MESSAGE)
        {
            Debug::log("Attempting login user: " + loginReqMsg.getUsername());
            const std::string usrname = loginReqMsg.getUsernameData();
            const std::string passwd = loginReqMsg.getPasswordData();
            std::string errorMessage = "";
            bool success = false;
            Faction userFaction = NULL_FACTION;

            QueryResult result = DatabaseManager::exec_query(
                "SELECT * FROM users WHERE name='" + usrname + "' AND password='" + passwd + "';"
            );
            if (result.status == QUERY_STATUS__SUCCESS)
            {
                if (result.result.size() == 0)
                {
                    Debug::log("Failed to login user due to invalid username or password");
                    errorMessage = "Invalid username or password";
                }
                else if (result.result.size() != 1)
                {
                    // NOTE: This should never be able to happen due to unique constraint
                    Debug::log(
                        "Multiple users found with name: " + usrname,
                        Debug::MessageType::FATAL_ERROR
                    );
                    return NULL_MESSAGE;
                }
                else
                {
                    if (result.getValue<bool>(0, DATABASE_COLUMN__USERS__LOGGED_IN))
                    {
                        Debug::log("Failed to login user due to user being logged in already");
                        errorMessage = "Failed to login";
                    }
                    else
                    {
                        std::string dbUserID = result.getValue<std::string>(0, DATABASE_COLUMN__USERS__ID);
                        std::string dbUsername = result.getValue<std::string>(0, DATABASE_COLUMN__USERS__NAME);
                        int tileX = result.getValue<int>(0, DATABASE_COLUMN__USERS__TILE_X);
                        int tileZ = result.getValue<int>(0, DATABASE_COLUMN__USERS__TILE_X);
                        User user(dbUserID, dbUsername.data(), dbUsername.size(), tileX, tileZ);

                        QueryResult setLoggedInResult = DatabaseManager::exec_query(
                            "UPDATE users SET logged_in=TRUE WHERE id='" + dbUserID + "';"
                        );
                        if (setLoggedInResult.status == QUERY_STATUS__SUCCESS)
                        {
                            server.loginUser(client, user);
                            success = true;

                            // Get user's faction if exists
                            QueryResult userFactionResult = DatabaseManager::exec_query(
                                "SELECT * FROM factions WHERE user_id = '" + dbUserID + "';"
                            );
                            if (userFactionResult.status == QUERY_STATUS__SUCCESS)
                            {
                                // NOTE: Currently user can have only one faction!
                                if (userFactionResult.result.size() > 1)
                                {
                                    Debug::log(
                                        "Failed to login user due to user having multiple factions. Currently this is prohibited",
                                        Debug::MessageType::FATAL_ERROR
                                    );
                                    success = false;
                                    errorMessage = "Internal server error";
                                }
                                else if (userFactionResult.result.size() == 1)
                                {
                                    std::string factionID = userFactionResult.getValue<std::string>(0, DATABASE_COLUMN__FACTIONS__ID);
                                    std::string factionName = userFactionResult.getValue<std::string>(0, DATABASE_COLUMN__FACTIONS__NAME);
                                    Debug::log("___TEST___FOUND FACTION: " + factionName + " for user");
                                    if (factionName.size() == 0 || factionName == "" || factionID.size() == 0 || factionID == "")
                                    {
                                        Debug::log(
                                            "Failed to login user due to user faction name or id being empty string",
                                            Debug::MessageType::FATAL_ERROR
                                        );
                                        success = false;
                                        errorMessage = "Internal server error";
                                    }
                                    else
                                    {
                                        userFaction = Faction(factionID, factionName);
                                        user.setFactionName(factionName);
                                    }
                                }
                            }
                            else
                            {
                                Debug::log(
                                    "Failed to login user due to error in Faction Query. Error message: " + userFactionResult.errorMsg,
                                    Debug::MessageType::FATAL_ERROR
                                );
                                success = false;
                                errorMessage = "Internal server error";
                            }

                        }
                        else
                        {
                            errorMessage = "Database error: Failed to assign user login status";
                        }
                    }
                }
            }
            else if(result.status == QUERY_STATUS__SYNTAX_ERROR)
            {
                Debug::log(
                    "User login failed due to query syntax error: " + result.errorMsg,
                    Debug::MessageType::ERROR
                );
                errorMessage = "Internal server error";
            }
            else if(result.status == QUERY_STATUS__CONNECTION_ERROR)
            {
                Debug::log(
                    "User login failed due to database connection error: " + result.errorMsg,
                    Debug::MessageType::ERROR
                );
                errorMessage = "Internal server error";
            }
            else if(result.status == QUERY_STATUS__UNEXPECTED_ERROR)
            {
                Debug::log(
                    "User login failed due to unexpected query error: " + result.errorMsg,
                    Debug::MessageType::ERROR
                );
                errorMessage = "Internal server error";
            }

            return LoginResponse(
                success,
                userFaction,
                errorMessage
            );
        }
        Debug::log("Failed to construct LoginRequest message from incoming data", Debug::MessageType::WARNING);
        return NULL_MESSAGE;
    }


    Message user_register(Server& server, const Client& client, Message& msg)
    {
        UserRegisterRequest registerReqMsg(msg.getData(), msg.getDataSize());
        if (registerReqMsg != NULL_MESSAGE)
        {
            const std::string usrname = registerReqMsg.getUsernameData();
            const std::string passwd = registerReqMsg.getPasswordData();
            const std::string repasswd = registerReqMsg.getRePasswordData();
            std::string errorMessage = "";
            bool success = false;

            // Validate received passwords match
            if (passwd != repasswd)
            {
                Debug::log("User registering failed due to received passwords didn't match");
                errorMessage = "Passwords needs to match";
            }
            else
            {
                QueryResult result = DatabaseManager::exec_query(
                    "INSERT INTO users(name, password) VALUES('" + usrname + "', '" + passwd + "');"
                );
                switch (result.status)
                {
                    case QUERY_STATUS__SUCCESS:
                        success = true;
                        break;
                    case QUERY_STATUS__UNIQUE_VIOLATION:
                        Debug::log("User registering failed due to username: " + usrname + " already exists");
                        errorMessage = "Invalid username";
                        break;
                    case QUERY_STATUS__SYNTAX_ERROR:
                        Debug::log("User registering failed due to query syntax error: " + result.errorMsg);
                        errorMessage = "Internal server error";
                        break;
                    case QUERY_STATUS__CONNECTION_ERROR:
                        Debug::log("User registering failed due to database connection error: " + result.errorMsg);
                        errorMessage = "Internal server error";
                        break;
                    case QUERY_STATUS__UNEXPECTED_ERROR:
                        Debug::log("User registering failed due to unexpected query error: " + result.errorMsg);
                        errorMessage = "Internal server error";
                        break;
                }
            }
            if (success)
            {
                Debug::log("New user registered: " + usrname + " name size=" + std::to_string(usrname.size()));
                // TODO: delete
                // server.createUser(client, usrname.data(), usrname.size(), passwd.data(), passwd.size());
            }
            return UserRegisterResponse(success, errorMessage);
        }
        return NULL_MESSAGE;
    }


    Message fetch_obj_type_lib(Server& server, const Client& client, Message& msg)
    {
        Debug::log("___TEST___User requested obj info lib!");
        return ObjInfoLibResponse(Game::get()->getObjInfoLib());
    }


    Message update_observer(Server& server, const Client& client, Message& msg)
    {
        const User user = server.getUser(client);
        if (user == NULL_USER)
        {
            Debug::log(
                "Attempted to update observer for client: " + client.getAddress() +
                " but client's user was nullptr",
                Debug::MessageType::ERROR
            );
            return NULL_MESSAGE;
        }

        UpdateObserverMsg updateMsg(msg.getData(), msg.getDataSize());
        if (updateMsg != NULL_MESSAGE)
        {
            server.updateUserData(client, updateMsg.getX(), updateMsg.getZ(), updateMsg.getRadius());
        }

        return NULL_MESSAGE;
    }


    //Message server_shutdown(Server& server, Message& msg)
    //{
    //    Server::trigger_shutdown();
    //    std::string message = "Shutdown triggered";
    //    return Message(NULL_CLIENT, message.data(), message.size());
    //}
}
