cmake_minimum_required(VERSION 3.16)
project(draw)

set(PROJECT_NAME draw)

include(qt.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt5 REQUIRED COMPONENTS Core)
find_package(Qt${QT_VERSION_MAJOR}
    COMPONENTS
        Core
        Gui
        Widgets
)
qt_standard_project_setup()

message("CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")

FILE(GLOB_RECURSE PROJECT_SOURCES 
src/*.cpp 
src/*.ui 
src/*.h )
message("PROJECT_SOURCES: ${PROJECT_SOURCES}")


set(PROJECT_SOURCES
    ${PROJECT_SOURCES}
)

qt_add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        WIN32_EXECUTABLE TRUE
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
)

