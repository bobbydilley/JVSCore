cmake_minimum_required(VERSION 3.10)

project(jvscore
        VERSION "1.4.0"
	LANGUAGES "C"
        DESCRIPTION "Linux user space device driver for JVS I/O boards"
        HOMEPAGE_URL "https://github.com/bobbydilley/JVSCore-Public"
)

find_package(Threads REQUIRED)

configure_file(include/version.h.in version.h)

include_directories(${THREAD_INLCUDE_DIRS} ${PROJECT_SOURCE_DIR}/include ${PROJECT_BINARY_DIR})

FILE(GLOB Sources src/*.c) #Assume we want to include everything from src/

add_executable(${PROJECT_NAME}
        ${Sources}
        docs/jvscore.conf
        docs/jvscore.service
)

set_target_properties(${PROJECT_NAME} PROPERTIES
        C_STANDARD 99
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -g)

target_link_libraries(${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT} m)

install(
        TARGETS ${PROJECT_NAME}
        COMPONENT binaries
        RUNTIME DESTINATION "bin/"
)

install(
        FILES docs/jvscore.conf
        COMPONENT config
        DESTINATION "/etc/"
)

install(
        FILES docs/jvscore.service
        DESTINATION "/lib/systemd/system/"
        COMPONENT config
)


SET(CPACK_GENERATOR "DEB")
SET(CPACK_DEBIAN_PACKAGE_MAINTAINER "bobby@dilley.uk") #required
INCLUDE(CPack)
