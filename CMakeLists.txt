cmake_minimum_required(VERSION 3.10)
project(MyLangCompiler CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Option to enable SSH functionality
# Default to ON, can be set to OFF with -DWITH_SSH=OFF during configuration
option(WITH_SSH "Build with SSH handling capabilities" ON)

set(SOURCES
    src/main.cpp
    src/Lexer.cpp
    src/Parser.cpp
    src/CLI.cpp
    src/Webserver.cpp
)

# Conditional compilation for SshHandler.cpp and SSH linking
if(WITH_SSH)
    message(STATUS "WITH_SSH is ON. Including SshHandler.cpp and attempting to link with libssh.")
    list(APPEND SOURCES src/SshHandler.cpp)

    # Try to find libssh using find_package first.
    # Many systems provide CMake configuration files for libssh, making this the most robust.
    # COMPONENTS can be used if specific parts of the library are needed, but for basic linking, it's often not strictly necessary.
    find_package(Libssh QUIET) # Use QUIET to avoid messages if not found immediately

    if(Libssh_FOUND)
        message(STATUS "Found Libssh using find_package.")
        # Libssh_INCLUDE_DIRS and Libssh_LIBRARIES are set by find_package
        include_directories(${Libssh_INCLUDE_DIRS})
        # Add a placeholder for the library, we'll link it with target_link_libraries
        set(SSH_LINK_LIBRARY ${Libssh_LIBRARIES})
    else()
        message(WARNING "find_package(Libssh) failed. Attempting to find 'ssh' library manually.")
        # Fallback if find_package doesn't work (e.g., no CMake config file for libssh)
        # Directly find the library file (e.g., libssh.so or libssh.a)
        find_library(SSH_LIBRARY_FILE ssh)

        if(NOT SSH_LIBRARY_FILE)
            message(FATAL_ERROR "Could not find 'ssh' library. Please ensure libssh development packages are installed (e.g., libssh-dev on Debian/Ubuntu, libssh-devel on Fedora/RHEL, brew install libssh on macOS). Set WITH_SSH to OFF if you don't need SSH support.")
        else()
            message(STATUS "Found SSH library file: ${SSH_LIBRARY_FILE}")
            set(SSH_LINK_LIBRARY ${SSH_LIBRARY_FILE})

            # Also try to find the headers for manual linking
            find_path(SSH_INCLUDE_DIR libssh/libssh.h
                      PATHS /usr/include /usr/local/include /opt/homebrew/include # Add common paths including Homebrew
                      )
            if(SSH_INCLUDE_DIR)
                message(STATUS "Found SSH include directory: ${SSH_INCLUDE_DIR}")
                include_directories(${SSH_INCLUDE_DIR})
            else()
                message(WARNING "Could not find libssh headers. Compilation might fail if direct SSH includes are used (e.g., #include <libssh/libssh.h>).")
            endif()
        endif()
    endif()
endif()

add_executable(my_lang_compiler ${SOURCES})

# Link the executable with the found SSH library if SSH is enabled and the library was found
if(WITH_SSH AND SSH_LINK_LIBRARY)
    message(STATUS "Linking my_lang_compiler with ${SSH_LINK_LIBRARY}")
    target_link_libraries(my_lang_compiler PRIVATE ${SSH_LINK_LIBRARY})
endif()