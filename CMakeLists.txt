# Set CMake version
cmake_minimum_required(VERSION 3.0)

# Set Project name
project(syncd)

# Set C Standard = C11
set(CMAKE_C_STANDARD 11)

# Set source files
set(SOURCE_FILES main.c parseconfig.c parseconfig.h configobject.c configobject.h net.c net.h actors.c actors.h)

# Include scripts to find required libraries
include(CMakeRecipes/Findhiredis.cmake)
include(CMakeRecipes/FindBSON.cmake)

find_library(MONGODB_LIBRARY mongoc-1.0)
find_library(BSON_LIBRARY bson-1.0)

# Include the libraries
include_directories(${BSON_INCLUDE_DIRS} ${MONGODB_LIBRARY_INCLUDE_DIRS} ${HIREDIS_INCLUDE_DIRS})

# Compile to executable
add_executable(syncd ${SOURCE_FILES})

# Finally, link BSON, Mongo, and Redis libraries
target_link_libraries(syncd ${MONGODB_LIBRARY} ${BSON_LIBRARIES} ${HIREDIS_LIBRARY})