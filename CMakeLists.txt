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
        src/Command_Layer/System_Commands/ConnCommand.hpp
        src/Command_Layer/System_Commands/PingCommand.hpp
        src/Command_Layer/System_Commands/ErrCommand.hpp
)
target_link_libraries(My2FA
        Qt::Core
        Qt::Gui
        Qt::Widgets
)

