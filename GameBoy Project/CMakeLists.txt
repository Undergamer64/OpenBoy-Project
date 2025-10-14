""cmake_minimum_required(VERSION 3.15)
project(GameBoyProject VERSION 1.0 LANGUAGES CXX)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# ==============================================
# VCPKG auto-detection and setup
# ==============================================

# Check if user already defined it (via command line)
if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    message(STATUS "No CMAKE_TOOLCHAIN_FILE specified. Trying to locate vcpkg...")

    # Option 1: Local vcpkg folder (inside project)
    if(EXISTS "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_TOOLCHAIN_FILE
            "${CMAKE_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "Vcpkg toolchain file")
        message(STATUS "Using local vcpkg from project folder")

    # Option 2: Global vcpkg installation (user-installed)
    elseif(DEFINED ENV{VCPKG_ROOT} AND EXISTS "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_TOOLCHAIN_FILE
            "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "Vcpkg toolchain file")
        message(STATUS "Using global vcpkg from $ENV{VCPKG_ROOT}")

    else()
        message(WARNING "⚠️ Could not find vcpkg automatically. Please configure CMake with:")
        message(WARNING "   -DCMAKE_TOOLCHAIN_FILE=<path-to-vcpkg>/scripts/buildsystems/vcpkg.cmake")
    endif()
endif()

# Find SFML package via vcpkg
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

# Add the executable
add_executable(${PROJECT_NAME} 
    GameBoy\ Project.cpp
    alu.cpp
    BootRom.cpp
    Cartridge.cpp
    cpu.cpp
    emulator.cpp
    mmu.cpp
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${SFML_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}
)

# Link SFML libraries
target_link_libraries(${PROJECT_NAME} sfml-graphics sfml-window sfml-system)

# Copy required assets to the build folder
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/dmg_boot.bin
        ${CMAKE_SOURCE_DIR}/dmg_decrypted.txt
        ${CMAKE_SOURCE_DIR}/Tetris.gb
        $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
""