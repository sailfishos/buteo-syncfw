cmake_minimum_required(VERSION 3.16)

project(buteo-syncfw)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED)

set(QT_VERSION_FOLDER "Qt${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}")

if(QT_VERSION_MAJOR EQUAL 6)
    set(CMAKE_CXX_STANDARD 17)
    find_package(Qt6 REQUIRED COMPONENTS
        Core
        Xml
        DBus
        Sql
        Network
    )
elseif(QT_VERSION_MAJOR EQUAL 5)
    set(CMAKE_CXX_STANDARD 11)
    add_compile_options(-fno-sized-deallocation)
    add_link_options(-fno-sized-deallocation)
    find_package(Qt5 REQUIRED COMPONENTS
        Core
        Xml
        DBus
        Sql
        Network
    )
endif()

add_subdirectory(libbuteosyncfw)
add_subdirectory(declarative)
add_subdirectory(msyncd)
add_subdirectory(oopp-runner)
add_subdirectory(unittests)
add_subdirectory(doc)
