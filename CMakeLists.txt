cmake_minimum_required(VERSION 3.5)

project(pay LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Ativa automoc, autouic e autorrc (boa prática com Qt5+)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Encontra os módulos Qt5 necessários
find_package(Qt5 REQUIRED COMPONENTS Widgets Charts)
find_package(OpenSSL REQUIRED)

# Lista de fontes do projeto
add_executable(pay
    main.cpp
    pay.cpp
    pay.h
    planner.cpp
    Planner/planner.hpp
    Planner/plannerwidget.h
    Planner/plannerwidget.cpp
    nubankreader.cpp
    nubankreader.h
)

# Vincula com as bibliotecas necessárias
target_link_libraries(pay
    Qt5::Widgets
    Qt5::Charts
    OpenSSL::Crypto
)
