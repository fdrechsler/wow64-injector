cmake_minimum_required(VERSION 3.10)
project(WoWLuaInjector)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Feature flags
option(ENABLE_PATTERN_SCANNING "Enable pattern scanning feature" ON)
option(ENABLE_WARDEN_DETECTION "Enable Warden detection feature" ON)
option(ENABLE_IAT_HIDING "Enable IAT hiding feature" ON)
option(ENABLE_THREAD_HIJACKING "Enable thread hijacking feature" ON)

# Version information
set(VERSION_MAJOR 1)
set(VERSION_MINOR 0)
set(VERSION_PATCH 0)

# Get current date
string(TIMESTAMP BUILD_DATE "%Y-%m-%d")

# Configure version header
configure_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/version.h.in"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/version.h"
    @ONLY
)

# Add compiler flags
if(MSVC)
    add_compile_options(
        /W4     # Warning level 4
        /WX     # Treat warnings as errors
        /MP     # Multi-processor compilation
        /GR-    # Disable RTTI
        /GF     # String pooling
        /Gy     # Function-level linking
        /Oi     # Intrinsic functions
        /GS     # Buffer security check
        /EHs-c- # Disable exceptions
        /sdl    # Security checks
    )
endif()

# Source files
set(SOURCES
    src/main.cpp
    src/warden_detector.cpp
    src/lua_injector.cpp
    src/iat_hiding.cpp
    src/pattern_scanner.cpp
    src/process_memory_handler.cpp
)

# Header files
set(HEADERS
    include/warden_detector.h
    include/pattern_scanner.h
    include/iat_hiding.h
    include/lua_injector.h
    include/process_memory_handler.h
    include/logger.h
    include/version.h
)

# Create executable
add_executable(injector ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(injector PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link libraries
target_link_libraries(injector PRIVATE
    ntdll.lib
    dbghelp.lib
)

# Set output directories
set_target_properties(injector PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/bin/Debug"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/bin/Release"
)

# Add preprocessor definitions
target_compile_definitions(injector PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

# Add feature flags to preprocessor definitions
if(ENABLE_PATTERN_SCANNING)
    target_compile_definitions(injector PRIVATE ENABLE_PATTERN_SCANNING)
endif()

if(ENABLE_WARDEN_DETECTION)
    target_compile_definitions(injector PRIVATE ENABLE_WARDEN_DETECTION)
endif()

if(ENABLE_IAT_HIDING)
    target_compile_definitions(injector PRIVATE ENABLE_IAT_HIDING)
endif()

if(ENABLE_THREAD_HIJACKING)
    target_compile_definitions(injector PRIVATE ENABLE_THREAD_HIJACKING)
endif()

# Set architecture-specific options
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    target_compile_definitions(injector PRIVATE _WIN64)
    set_target_properties(injector PROPERTIES OUTPUT_NAME "injector64")
else()
    set_target_properties(injector PROPERTIES OUTPUT_NAME "injector32")
endif()

# Set optimization flags for Release build
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(MSVC)
        add_compile_options(/O2 /GL)
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LTCG")
    endif()
endif()

# Install targets
install(TARGETS injector
    RUNTIME DESTINATION bin
)

# Add custom target for cleaning all build artifacts
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -P "${CMAKE_CURRENT_SOURCE_DIR}/cmake/clean-all.cmake"
)