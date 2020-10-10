cmake_minimum_required(VERSION 3.13)
project(openni_player CXX)


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt5 COMPONENTS Core Gui Widgets Multimedia MultimediaWidgets REQUIRED)
find_library(openni2 REQUIRED)


set(openni_player_SRC_DIR "${PROJECT_SOURCE_DIR}/sources")
set(openni_player_SRC ${openni_player_SRC_DIR}/main.cpp
                      ${openni_player_SRC_DIR}/videoplayer.hpp
                      ${openni_player_SRC_DIR}/videoplayer.cpp
                      ${openni_player_SRC_DIR}/onistream.hpp
                      ${openni_player_SRC_DIR}/onistream.cpp)


add_executable(openni_player ${openni_player_SRC})
#target_include_directories(openni_player PRIVATE )
target_link_libraries(openni_player openni2
                                    Qt5::Core Qt5::Widgets Qt5::Gui Qt5::Multimedia Qt5::MultimediaWidgets)
target_compile_features(openni_player PRIVATE cxx_std_17)
