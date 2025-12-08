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

add_executable(My2FA main.cpp
        src/My2FA_Server/server.cpp
        src/Command_Layer/Base/Command.hpp
        src/Command_Layer/Base/CommandTypes.hpp
        src/Command_Layer/CommandFactory.cpp
        src/Command_Layer/CommandFactory.hpp
        src/Command_Layer/System_Commands/ConnectCommand.hpp
        src/Command_Layer/System_Commands/PingCommand.hpp
        src/Command_Layer/System_Commands/ErrorCommand.hpp
        src/Command_Layer/Credential_Login/LoginRequestCommand.hpp
        src/Command_Layer/Credential_Login/LoginResponseCommand.hpp
        src/Command_Layer/Notification_Login/RequestNotificationClientCommand.hpp
        src/Command_Layer/Notification_Login/RequestNotificationServerCommand.hpp
        src/Command_Layer/Notification_Login/SendNotificationCommand.hpp
        src/Command_Layer/Notification_Login/NotificationResponseClientCommand.hpp
        src/Command_Layer/Notification_Login/NotificationResponseServerCommand.hpp
        src/Command_Layer/Code_Login/RequestCodeClientCommand.hpp
        src/Command_Layer/Code_Login/CodeResponseCommand.hpp
        src/Command_Layer/Code_Login/ValidateCodeClientCommand.hpp
        src/Command_Layer/Code_Login/ValidateCodeServerCommand.hpp
        src/Command_Layer/Code_Login/ValidateResponseServerCommand.hpp
        src/Command_Layer/Code_Login/ValidateResponseClientCommand.hpp
)
target_link_libraries(My2FA
        Qt::Core
        Qt::Gui
        Qt::Widgets
)

