# Set CMake version
cmake_minimum_required(VERSION 3.0)

# Set Project name
project(magicsync)

# Set C Standard = C11
set(CMAKE_C_STANDARD 11)

# Set source files
set(SOURCE_FILES main.c parseconfig.c parseconfig.h configobject.c configobject.h pushactions.c pushactions.h pullactions.c pullactions.h net.c net.h)

# Include scripts to find required libraries
include(Findhiredis.cmake)
include(FindBSON.cmake)

find_library(MONGODB_LIBRARY mongoc-1.0)
find_library(BSON_LIBRARY bson-1.0)

# Include the libraries
include_directories(${BSON_INCLUDE_DIRS} ${MONGODB_LIBRARY_INCLUDE_DIRS} ${HIREDIS_INCLUDE_DIRS})

# Compile to executable
add_executable(magicsync ${SOURCE_FILES})

# Finally, link BSON, Mongo, and Redis libraries
target_link_libraries(magicsync ${MONGODB_LIBRARY} ${BSON_LIBRARIES} ${HIREDIS_LIBRARY})