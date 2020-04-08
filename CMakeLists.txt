cmake_minimum_required(VERSION 3.10)

project(jvscore)

find_package(Threads REQUIRED)

include_directories(${THREAD_INLCUDE_DIRS} ${PROJECT_SOURCE_DIR}/include)

FILE(GLOB Sources src/*.c) #Assume we want to include everything from src/

add_executable(${PROJECT_NAME}
        ${Sources}
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 99
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})

install(TARGETS ${PROJECT_NAME}
	COMPONENT {$PROJECT_NAME}
    RUNTIME DESTINATION "bin/"
    LIBRARY DESTINATION "lib/"
    DESTINATION "/usr/"
)

SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "bobby@dilley.uk") #required
INCLUDE(CPack)