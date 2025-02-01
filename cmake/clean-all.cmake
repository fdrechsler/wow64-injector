# Clean all build artifacts

# Function to remove directory and its contents if it exists
function(remove_directory_if_exists dir)
    if(EXISTS "${dir}")
        file(REMOVE_RECURSE "${dir}")
    endif()
endfunction()

# Function to remove file if it exists
function(remove_file_if_exists file)
    if(EXISTS "${file}")
        file(REMOVE "${file}")
    endif()
endfunction()

# Remove build directories
remove_directory_if_exists("${CMAKE_BINARY_DIR}/build32")
remove_directory_if_exists("${CMAKE_BINARY_DIR}/build64")
remove_directory_if_exists("${CMAKE_BINARY_DIR}/bin")
remove_directory_if_exists("${CMAKE_BINARY_DIR}/dist")

# Remove CMake cache and files
remove_file_if_exists("${CMAKE_BINARY_DIR}/CMakeCache.txt")
remove_directory_if_exists("${CMAKE_BINARY_DIR}/CMakeFiles")
remove_file_if_exists("${CMAKE_BINARY_DIR}/cmake_install.cmake")
remove_file_if_exists("${CMAKE_BINARY_DIR}/install_manifest.txt")

# Remove Visual Studio files
remove_file_if_exists("${CMAKE_BINARY_DIR}/*.sln")
remove_file_if_exists("${CMAKE_BINARY_DIR}/*.vcxproj")
remove_file_if_exists("${CMAKE_BINARY_DIR}/*.vcxproj.filters")
remove_file_if_exists("${CMAKE_BINARY_DIR}/*.vcxproj.user")

# Remove generated version header
remove_file_if_exists("${CMAKE_SOURCE_DIR}/include/version.h")

# Remove any temporary or backup files
file(GLOB_RECURSE temp_files
    "${CMAKE_SOURCE_DIR}/*.tmp"
    "${CMAKE_SOURCE_DIR}/*~"
    "${CMAKE_SOURCE_DIR}/*.bak"
)
foreach(temp_file ${temp_files})
    remove_file_if_exists("${temp_file}")
endforeach()

message(STATUS "All build artifacts have been cleaned.")