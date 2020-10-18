cmake_minimum_required(VERSION 3.13)
project(openni_player C CXX)


set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)


find_package(Qt5 COMPONENTS Core Widgets Multimedia MultimediaWidgets REQUIRED)


if(UNIX AND NOT APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()


set(third_party_DIR "${PROJECT_SOURCE_DIR}/third_party")


# ---------- XnLib ----------
set(XnLib_DIR "${third_party_DIR}/XnLib")
file(GLOB XnLib_SRC "${XnLib_DIR}/Source/*.cpp")

if(WIN32)
    file(GLOB XnLib_WIN32_SRC "${XnLib_DIR}/Source/Win32/*.cpp")
    list(APPEND XnLib_SRC ${XnLib_WIN32_SRC})
elseif(UNIX AND NOT APPLE)
    file(GLOB XnLib_LINUX_SRC "${XnLib_DIR}/Source/Linux/*.cpp")
    list(APPEND XnLib_SRC ${XnLib_LINUX_SRC})
else()
    message("ERROR: Not supported OS")
endif()

add_library(XnLib STATIC ${XnLib_SRC})

target_include_directories(XnLib PUBLIC "${XnLib_DIR}/Include")
if(WIN32)
    target_include_directories(XnLib PRIVATE "${XnLib_DIR}/Driver/Win32/Build/sys")
endif()

target_compile_definitions(XnLib PRIVATE XnLib_EXPORT)
set_target_properties(XnLib PROPERTIES LINKER_LANGUAGE CXX)


# ---------- LibJPEG ----------
set(LibJPEG_DIR "${third_party_DIR}/LibJPEG")
file(GLOB LibJPEG_SRC "${LibJPEG_DIR}/*.c")


# ---------- OpenNI2 ----------
set(OpenNI2_DIR "${third_party_DIR}/OpenNI2")
set(OpenNI2_INCLUDE_DIR "${OpenNI2_DIR}/Include")
set(OpenNI2_FORMATS_INCLUDE_DIR "${OpenNI2_DIR}/Source/Drivers/OniFile/Formats")

file(GLOB OpenNI2_CORE_SRC "${OpenNI2_DIR}/Source/Core/*.cpp")
file(GLOB OpenNI2_FORMATS_SRC "${OpenNI2_DIR}/Source/Drivers/OniFile/Formats/*.cpp")

set(OpenNI2_SRC ${OpenNI2_CORE_SRC} ${OpenNI2_FORMATS_SRC})

add_library(OpenNI2 SHARED ${OpenNI2_SRC} ${LibJPEG_SRC})
target_link_libraries(OpenNI2 PRIVATE XnLib)
target_include_directories(OpenNI2 PUBLIC ${OpenNI2_INCLUDE_DIR}
                                   PRIVATE ${OpenNI2_FORMATS_INCLUDE_DIR} ${LibJPEG_DIR})
target_compile_definitions(OpenNI2 PRIVATE OPENNI2_EXPORT)


# ---------- openni_player ----------
set(openni_player_SRC_DIR "${PROJECT_SOURCE_DIR}/sources")
set(openni_player_SRC ${openni_player_SRC_DIR}/main.cpp
                      ${openni_player_SRC_DIR}/OniPlayer.hpp
                      ${openni_player_SRC_DIR}/OniPlayer.cpp
                      ${openni_player_SRC_DIR}/OniFrameProvider.hpp
                      ${openni_player_SRC_DIR}/OniFrameProvider.cpp
                      ${openni_player_SRC_DIR}/OniFrameSource.hpp
                      ${openni_player_SRC_DIR}/OniFrameSource.cpp)


add_executable(openni_player ${openni_player_SRC})
target_link_libraries(openni_player Qt5::Core Qt5::Widgets Qt5::Multimedia Qt5::MultimediaWidgets
                                    OpenNI2)
target_compile_features(openni_player PRIVATE cxx_std_17)


# ---------- OpenNI2 OniFile ----------
set(OniFile_DIR "${OpenNI2_DIR}/Source/Drivers/OniFile")

file(GLOB OniFile_SRC "${OniFile_DIR}/*.cpp")

add_library(OniFile SHARED ${OniFile_SRC} ${OpenNI2_FORMATS_SRC} ${LibJPEG_SRC})
target_link_libraries(OniFile PRIVATE XnLib)
target_include_directories(OniFile PRIVATE ${OniFile_DIR} ${OpenNI2_INCLUDE_DIR} ${OpenNI2_FORMATS_INCLUDE_DIR} ${LibJPEG_DIR})
set_target_properties(OniFile PROPERTIES RUNTIME_OUTPUT_DIRECTORY $<TARGET_FILE_DIR:openni_player>/OpenNI2/Drivers/)
