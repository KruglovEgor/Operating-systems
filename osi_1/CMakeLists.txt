cmake_minimum_required(VERSION 3.20)
project(osi_1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(/EHsc)

add_executable(osi_1 src/Main.cpp
        src/CommandExecutor.cpp
        src/CommandExecutor.h
        src/Shell.cpp
        src/Shell.h
        src/Commands/CatCommand.h
        src/Commands/HistoryCommand.h
        src/Commands/MkdirCommand.h
        src/Commands/HelpCommand.h
        src/Commands/RmCommand.h
        src/Commands/TouchCommand.h)

add_executable(io-thpt-write src/Benchmarks/IoThptWrite.cpp)
add_executable(sort src/Benchmarks/Sort.cpp)


# Подключение библиотеки ntdll
target_link_libraries(osi_1 PRIVATE ntdll.lib)
