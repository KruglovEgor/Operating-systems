cmake_minimum_required(VERSION 3.15)
project(cache_library)

set(CMAKE_CXX_STANDARD 17)

# Указываем путь к директории с заголовочными файлами
include_directories("include")

# Список исходных файлов
set(SOURCE_FILES
        src/api.cpp
        src/lru_policy.cpp
        test.cpp
)

# Создаем исполнимый файл для тестов
add_executable(cache_library_test ${SOURCE_FILES})

add_library(cache_library SHARED src/api.cpp src/lru_policy.cpp)

target_include_directories(cache_library PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

