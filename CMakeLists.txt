cmake_minimum_required(VERSION 4.0)
project(My2FA)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "/home/lynyx/Qt/6.10.1/gcc_64")

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

# Shared CommandLayer library so that we don't build it or write the config multiple times
add_library(CommandLayerLib STATIC
        src/Command_Layer/CommandFactory.hpp
        src/Command_Layer/CommandFactory.cpp

        src/Command_Layer/Base/Command.hpp
        src/Command_Layer/Base/CommandTypes.hpp

        src/Command_Layer/System_Commands/SystemCommands.hpp
        src/Command_Layer/System_Commands/ConnectCommand.hpp
        src/Command_Layer/System_Commands/ErrorCommand.hpp
        src/Command_Layer/System_Commands/PingCommand.hpp

        src/Command_Layer/Credential_Login/CredentialLoginCommands.hpp
        src/Command_Layer/Credential_Login/LoginRequestCommand.hpp
        src/Command_Layer/Credential_Login/LoginResponseCommand.hpp

        src/Command_Layer/Notification_Login/NotificationLoginCommands.hpp
        src/Command_Layer/Notification_Login/RequestNotificationClientCommand.hpp
        src/Command_Layer/Notification_Login/RequestNotificationServerCommand.hpp
        src/Command_Layer/Notification_Login/SendNotificationCommand.hpp
        src/Command_Layer/Notification_Login/NotificationResponseClientCommand.hpp
        src/Command_Layer/Notification_Login/NotificationResponseServerCommand.hpp

        src/Command_Layer/Code_Login/CodeLoginCommands.hpp
        src/Command_Layer/Code_Login/RequestCodeClientCommand.hpp
        src/Command_Layer/Code_Login/CodeResponseCommand.hpp
        src/Command_Layer/Code_Login/ValidateCodeClientCommand.hpp
        src/Command_Layer/Code_Login/ValidateCodeServerCommand.hpp
        src/Command_Layer/Code_Login/ValidateResponseServerCommand.hpp
        src/Command_Layer/Code_Login/ValidateResponseClientCommand.hpp
)

target_include_directories(CommandLayerLib PUBLIC src)

target_link_libraries(CommandLayerLib PRIVATE Qt::Core)

# AS executable
add_executable(AuthServer
        src/My2FA_Server/AuthServer.cpp
)

# Link to the lib for AS
target_link_libraries(AuthServer PRIVATE
        CommandLayerLib
        Qt::Core
)

add_executable(AuthClient
        src/My2FA_Client/AuthClient.cpp
        src/Connection_Layer/ClientConnectionHandler.hpp
)

target_link_libraries(AuthClient PRIVATE
        CommandLayerLib
        Qt::Core
)

# Test Client
#add_executable(TestClient
#        src/Test_Client/TestClient.cpp
#)
#
#target_link_libraries(TestClient PRIVATE
#        CommandLayerLib
#        Qt::Core

add_executable(DummyServer
        src/Dummy_Server/DummyServer.cpp)

target_link_libraries(DummyServer PRIVATE
        CommandLayerLib
        Qt::Core
)
add_executable(DummyClient
        src/Dummy_Client/DummyClient.cpp
        src/Connection_Layer/ClientConnectionHandler.hpp)

target_link_libraries(DummyClient PRIVATE
        CommandLayerLib
        Qt::Core
)