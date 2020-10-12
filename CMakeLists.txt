cmake_minimum_required(VERSION 3.13)
project(openni_player CXX)


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# С vcpkg попроще было бы
if(DEFINED ENV{OPENNI2_LIB64})
    set(openni2_LIB_DIR "$ENV{OPENNI2_LIB64}")
elseif(DEFINED ENV{OPENNI2_REDIST})
    set(openni2_LIB_DIR "$ENV{OPENNI2_REDIST}")
else()
    message("OpenNI2 lib dir not found")
endif()

if(DEFINED ENV{OPENNI2_INCLUDE64})
    set(openni2_INCLUDE_DIR "$ENV{OPENNI2_INCLUDE64}")
elseif(DEFINED ENV{OPENNI2_INCLUDE})
    set(openni2_INCLUDE_DIR "$ENV{OPENNI2_INCLUDE}")
else()
    message("OpenNI2 include dir not found")
endif()


find_package(Qt5 COMPONENTS Core Widgets Multimedia MultimediaWidgets REQUIRED)
find_library(OpenNI2 NAMES OpenNI2 PATHS "${openni2_LIB_DIR}" REQUIRED)
find_path(openni2_INCLUDE_PATH OpenNI.h PATHS ${openni2_INCLUDE_DIR})


set(openni_player_SRC_DIR "${PROJECT_SOURCE_DIR}/sources")
set(openni_player_SRC ${openni_player_SRC_DIR}/main.cpp
                      ${openni_player_SRC_DIR}/OniPlayer.hpp
                      ${openni_player_SRC_DIR}/OniPlayer.cpp
                      ${openni_player_SRC_DIR}/OniFrameProvider.hpp
                      ${openni_player_SRC_DIR}/OniFrameProvider.cpp
                      ${openni_player_SRC_DIR}/OniFrameSource.hpp
                      ${openni_player_SRC_DIR}/OniFrameSource.cpp)


add_executable(openni_player ${openni_player_SRC})
target_include_directories(openni_player PRIVATE ${openni2_INCLUDE_PATH})
target_link_directories(openni_player PRIVATE ${openni2_LIB_DIR})
target_link_libraries(openni_player Qt5::Core Qt5::Widgets Qt5::Multimedia Qt5::MultimediaWidgets
                                    OpenNI2)
target_compile_features(openni_player PRIVATE cxx_std_17)

if(DEFINED ENV{OPENNI2_REDIST64})
    add_custom_command(TARGET openni_player POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy
                           $ENV{OPENNI2_REDIST64}/OpenNI2.dll
                           $<TARGET_FILE_DIR:openni_player>/OpenNI2.dll)
    add_custom_command(TARGET openni_player POST_BUILD
                       COMMAND ${CMAKE_COMMAND} -E copy_directory
                           $ENV{OPENNI2_REDIST64}/OpenNI2
                           $<TARGET_FILE_DIR:openni_player>/OpenNI2)
endif()
