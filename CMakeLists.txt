cmake_minimum_required(VERSION 3.16)
project(MedicalImageViewer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable Qt's automatic processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Find required packages
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Try to find GDCM with more explicit configuration
find_package(GDCM REQUIRED COMPONENTS gdcmCommon gdcmIOD gdcmMSFF)

# Source files
set(SOURCES
    main.cpp
    mainwindow.cpp
    imageviewer.cpp
    dicomloader.cpp
    annotationmanager.cpp
)

set(HEADERS
    mainwindow.h
    imageviewer.h
    dicomloader.h
    annotationmanager.h
)

# Create executable
add_executable(MedicalImageViewer ${SOURCES} ${HEADERS})

# Link libraries with more explicit GDCM linking
target_link_libraries(MedicalImageViewer
    Qt6::Core
    Qt6::Widgets
    gdcmCommon
    gdcmIOD
    gdcmMSFF
)

# Include GDCM headers
if(GDCM_INCLUDE_DIRS)
    target_include_directories(MedicalImageViewer PRIVATE ${GDCM_INCLUDE_DIRS})
endif()
