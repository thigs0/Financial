cmake_minimum_required(VERSION 3.5)

project(pay LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets)
find_package(OpenSSL REQUIRED)

set(CMAKE_AUTOMOC ON)  # ← Isso ativa o moc automaticamente

add_executable(pay
    main.cpp
    pay.cpp
    nubankreader.cpp
    pay.h
    nubankreader.h
)

target_link_libraries(pay
    Qt5::Widgets
    OpenSSL::Crypto
)
