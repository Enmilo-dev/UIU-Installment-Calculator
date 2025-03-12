cmake_minimum_required(VERSION 3.26)
project(UIU_Installment_Calculator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)        # Critical for Qt meta-object system
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Explicit Qt path configuration
set(CMAKE_PREFIX_PATH "D:/qt/6.8.2/mingw_64/lib/cmake/Qt6")

find_package(Qt6 COMPONENTS Widgets REQUIRED)

# List all source files explicitly
add_executable(${PROJECT_NAME}
        main.cpp
        resources.qrc
        ${APP_ICON_RESOURCE_WINDOWS}
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE Qt6::Widgets
)