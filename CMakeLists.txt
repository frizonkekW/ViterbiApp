cmake_minimum_required(VERSION 3.10)
project(ViterbiSimulation)

# Стандарт C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Оптимизация для работы с тяжелыми вычислениями (декодер)
if(NOT MSVC)
    add_compile_options(-O3 -Wall)
else()
    add_compile_options(/O2 /W4)
endif()

# Указываем, где искать заголовочные файлы (.h)
# Это позволяет в .cpp файлах писать #include "BscChannel.h" без путей
include_directories(include)

# Собираем список всех исходных файлов в папке src
file(GLOB SOURCES "src/*.cpp")

# Создаем исполняемый файл
add_executable(viterbi_app ${SOURCES})