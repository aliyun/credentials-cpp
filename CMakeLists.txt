cmake_minimum_required(VERSION 3.10)

project(alibabacloud_credential VERSION "0.1.0")

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< General set up >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> #
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-specific compiler flags
if(MSVC)
    # Windows MSVC compiler settings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3")
    add_definitions(-D_CRT_SECURE_NO_WARNINGS)
    add_definitions(-DNOMINMAX)
    # Force all targets to use dynamic runtime library (MD) to avoid linker conflicts
    # This must be set before any targets are created
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    # Also set for C compiler
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /MD")
    set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /MDd")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MD")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MDd")
    # Fix __cplusplus macro to report the correct C++ standard version
    # Without this, MSVC always reports 199711L (C++98) regardless of actual standard
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
else()
    # GCC/Clang compiler settings (Linux/macOS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall")
    if(APPLE)
        # macOS specific flags
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")
    endif()
endif()

message("C++ compiler flags: ${CMAKE_CXX_FLAGS}")

if (CMAKE_CXX_COMPILER_LOADED)
    message(STATUS "The C++ compiler ID is: ${CMAKE_CXX_COMPILER_ID}")
    message(STATUS "The C++ compiler version is: ${CMAKE_CXX_COMPILER_VERSION}")
endif ()

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif ()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Target set up >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> #
# add_library(${PROJECT_NAME} SHARED "")
# 收集 include/AlibabaCloud 目录下的所有 .hpp 文件
include_directories(${CMAKE_SOURCE_DIR}/include)

set(SOURCE_FILES
        src/Client.cpp
        src/AuthUtil.cpp
        src/Constant.cpp
        src/Model.cpp
        src/provider/RefreshableProvider.cpp
        src/provider/DefaultProvider.cpp
        src/provider/EcsRamRoleProvider.cpp
        src/provider/EnvironmentVariableProvider.cpp
        src/provider/OIDCRoleArnProvider.cpp
        src/provider/ProfileProvider.cpp
        src/provider/CLIProfileProvider.cpp
        src/provider/RamRoleArnProvider.cpp
        src/provider/RsaKeyPairProvider.cpp
        src/provider/URLProvider.cpp
        src/provider/CloudSSOCredentialsProvider.cpp
        src/provider/OAuthCredentialsProvider.cpp)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

# Ensure main library uses dynamic runtime library on Windows
if(MSVC)
    # MSVC_RUNTIME_LIBRARY property requires CMake 3.15+
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.15")
        set_target_properties(${PROJECT_NAME} PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    endif()
    
    # Explicitly exclude static C and C++ runtime libraries to prevent conflicts
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.13")
        target_link_options(${PROJECT_NAME} PRIVATE
            /NODEFAULTLIB:libcmt
            /NODEFAULTLIB:libcpmt
            "$<$<CONFIG:Debug>:/NODEFAULTLIB:libcmtd>"
            "$<$<CONFIG:Debug>:/NODEFAULTLIB:libcpmtd>")
    else()
        # For older CMake versions, use LINK_FLAGS
        get_target_property(_current_link_flags ${PROJECT_NAME} LINK_FLAGS)
        if(NOT _current_link_flags OR _current_link_flags STREQUAL "NOTFOUND")
            set(_current_link_flags "")
        endif()
        set_target_properties(${PROJECT_NAME} PROPERTIES
            LINK_FLAGS "${_current_link_flags} /NODEFAULTLIB:libcmt /NODEFAULTLIB:libcpmt")
    endif()
endif()

target_include_directories(${PROJECT_NAME}
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Platform-specific library settings
set_target_properties(${PROJECT_NAME}
        PROPERTIES
        POSITION_INDEPENDENT_CODE 1
        SOVERSION ${PROJECT_VERSION_MAJOR}
        OUTPUT_NAME "${PROJECT_NAME}"
        DEBUG_POSTFIX "_d"
        PUBLIC_HEADER "${headers}"
        MACOSX_RPATH ON
        WINDOWS_EXPORT_ALL_SYMBOLS ON)

# Windows specific: handle DLL export
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE BUILDING_DLL)
endif()

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< External set up >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> #
# Ensure GoogleTest uses the shared CRT (MD) to match the project
# This must be set BEFORE googletest is configured
if(MSVC AND ENABLE_UNIT_TESTS)
    # Force GoogleTest to use shared CRT - must be set before add_subdirectory
    set(gtest_force_shared_crt ON CACHE BOOL "Force GoogleTest to use shared CRT" FORCE)
    
    # Ensure this propagates to subdirectories
    set(CMAKE_POLICY_DEFAULT_CMP0091 NEW)
    
    # Set global MSVC runtime library for all targets in subdirectories
    if(POLICY CMP0091)
        cmake_policy(SET CMP0091 NEW)
    endif()
endif()
add_subdirectory(external)

# Find OpenSSL with platform-specific hints
if(WIN32)
    # Windows: help find OpenSSL (can be installed via vcpkg or chocolatey)
    set(OPENSSL_USE_STATIC_LIBS FALSE)
endif()
find_package(OpenSSL REQUIRED)

if(OPENSSL_FOUND)
    message(STATUS "OpenSSL version: ${OPENSSL_VERSION}")
    message(STATUS "OpenSSL include dir: ${OPENSSL_INCLUDE_DIR}")
    message(STATUS "OpenSSL libraries: ${OPENSSL_LIBRARIES}")
else()
    message(FATAL_ERROR "OpenSSL not found. Please install OpenSSL:")
    message(FATAL_ERROR "  - Windows: vcpkg install openssl or choco install openssl")
    message(FATAL_ERROR "  - macOS: brew install openssl")
    message(FATAL_ERROR "  - Linux: sudo apt-get install libssl-dev")
endif()

# Find nlohmann_json for CLIProfileProvider JSON support
find_package(nlohmann_json QUIET)
if(nlohmann_json_FOUND)
    message(STATUS "Found nlohmann_json, enabling JSON support for CLIProfileProvider")
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_NLOHMANN_JSON)
    target_link_libraries(${PROJECT_NAME} PRIVATE darabonba_core OpenSSL::SSL OpenSSL::Crypto nlohmann_json::nlohmann_json)
else()
    message(WARNING "nlohmann_json not found, CLIProfileProvider will only support INI format")
    message(STATUS "To enable JSON support, install nlohmann_json:")
    message(STATUS "  - Windows: vcpkg install nlohmann-json")
    message(STATUS "  - Ubuntu/Debian: sudo apt-get install nlohmann-json3-dev")
    message(STATUS "  - macOS: brew install nlohmann-json")
    message(STATUS "  - Or add to external dependencies")
    target_link_libraries(${PROJECT_NAME} PRIVATE darabonba_core OpenSSL::SSL OpenSSL::Crypto)
endif()

# Platform-specific link libraries
if(WIN32)
    # Windows: link against ws2_32 and other system libraries if needed
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32)
    
    # Copy vcpkg DLLs to output directory for runtime
    if(DEFINED VCPKG_INSTALLED_DIR)
        set(_vcpkg_bin_dir "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin")
        if(EXISTS "${_vcpkg_bin_dir}")
            # Copy dependency DLLs to the same directory as the built library
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${_vcpkg_bin_dir}/libcurl.dll"
                $<TARGET_FILE_DIR:${PROJECT_NAME}>
                COMMENT "Copying libcurl.dll to output directory")
            
            # Copy OpenSSL DLLs
            file(GLOB _ssl_dlls 
                "${_vcpkg_bin_dir}/libcrypto*.dll"
                "${_vcpkg_bin_dir}/libssl*.dll")
            foreach(_dll ${_ssl_dlls})
                add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${_dll}"
                    $<TARGET_FILE_DIR:${PROJECT_NAME}>
                    COMMENT "Copying SSL DLL to output directory")
            endforeach()
        endif()
    endif()
elseif(UNIX AND NOT APPLE)
    # Linux: link against pthread and dl
    target_link_libraries(${PROJECT_NAME} PRIVATE pthread dl)
elseif(APPLE)
    # macOS: additional frameworks if needed
    # target_link_libraries(${PROJECT_NAME} PRIVATE "-framework CoreFoundation")
endif()

if (ENABLE_UNIT_TESTS)
    enable_testing()
    
    # Get darabonba_core source directory
    include(FetchContent)
    FetchContent_GetProperties(_darabonba_core)
    if(NOT _darabonba_core_POPULATED)
        FetchContent_Populate(_darabonba_core)
    endif()
    
    set(TEST_SOURCE_FILES
        tests/test_basic.cpp
        tests/test_config_model.cpp
        tests/test_providers.cpp
        tests/test_environment_default.cpp
        tests/test_url_provider.cpp
        tests/test_profile_provider.cpp
        tests/test_client.cpp
        tests/test_auth_util.cpp
        tests/test_cli_profile_provider.cpp
        tests/test_refreshable_provider.cpp
        tests/test_edge_cases.cpp
        tests/test_integration_scenarios.cpp
        tests/test_ecs_ram_role_provider.cpp
        tests/test_env_priority.cpp)
    
    add_executable(tests_AlibabaCloud_credential ${TEST_SOURCE_FILES})
    
    # Ensure test executable uses dynamic runtime library on Windows
    if(MSVC)
        # MSVC_RUNTIME_LIBRARY property requires CMake 3.15+
        if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.15")
            set_target_properties(tests_AlibabaCloud_credential PROPERTIES
                MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
        endif()
        
        # Explicitly exclude static C++ runtime libraries to prevent conflicts
        # libcmt.lib = static C runtime (Release)
        # libcpmt.lib = static C++ runtime (Release)
        # libcmtd.lib = static C runtime (Debug)
        # libcpmtd.lib = static C++ runtime (Debug)
        if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.13")
            target_link_options(tests_AlibabaCloud_credential PRIVATE
                /NODEFAULTLIB:libcmt
                /NODEFAULTLIB:libcpmt
                "$<$<CONFIG:Debug>:/NODEFAULTLIB:libcmtd>"
                "$<$<CONFIG:Debug>:/NODEFAULTLIB:libcpmtd>")
        else()
            # For older CMake versions, use LINK_FLAGS
            get_target_property(_current_link_flags tests_AlibabaCloud_credential LINK_FLAGS)
            if(NOT _current_link_flags OR _current_link_flags STREQUAL "NOTFOUND")
                set(_current_link_flags "")
            endif()
            set_target_properties(tests_AlibabaCloud_credential PROPERTIES
                LINK_FLAGS "${_current_link_flags} /NODEFAULTLIB:libcmt /NODEFAULTLIB:libcpmt")
        endif()
    endif()
    
    target_include_directories(tests_AlibabaCloud_credential
            PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/include
            ${_darabonba_core_SOURCE_DIR}/include)
    
    target_link_libraries(tests_AlibabaCloud_credential
            PRIVATE
            ${PROJECT_NAME}
            gtest
            gtest_main
            darabonba_core
            OpenSSL::SSL
            OpenSSL::Crypto)
    
    # Platform-specific test link libraries
    if(WIN32)
        target_link_libraries(tests_AlibabaCloud_credential PRIVATE ws2_32)
        
        # Copy darabonba_core.dll to the executable directory for Windows
        add_custom_command(TARGET tests_AlibabaCloud_credential POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:darabonba_core>
            $<TARGET_FILE_DIR:tests_AlibabaCloud_credential>
            COMMENT "Copying darabonba_core.dll to output directory")
        
        # Copy vcpkg DLLs (libcurl, OpenSSL, etc.) to output directory
        if(DEFINED VCPKG_INSTALLED_DIR)
            set(_vcpkg_bin_dir "${VCPKG_INSTALLED_DIR}/${VCPKG_TARGET_TRIPLET}/bin")
            if(EXISTS "${_vcpkg_bin_dir}")
                file(GLOB _vcpkg_dlls 
                    "${_vcpkg_bin_dir}/libcurl*.dll"
                    "${_vcpkg_bin_dir}/libcrypto*.dll"
                    "${_vcpkg_bin_dir}/libssl*.dll"
                    "${_vcpkg_bin_dir}/zlib*.dll"
                    "${_vcpkg_bin_dir}/nghttp2*.dll"
                    "${_vcpkg_bin_dir}/brotli*.dll"
                    "${_vcpkg_bin_dir}/zstd*.dll")
                foreach(_dll ${_vcpkg_dlls})
                    add_custom_command(TARGET tests_AlibabaCloud_credential POST_BUILD
                        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${_dll}"
                        $<TARGET_FILE_DIR:tests_AlibabaCloud_credential>
                        COMMENT "Copying ${_dll} to output directory")
                endforeach()
            endif()
        endif()
    elseif(UNIX AND NOT APPLE)
        target_link_libraries(tests_AlibabaCloud_credential PRIVATE pthread dl)
    endif()
    
    add_test(NAME google_test
            COMMAND $<TARGET_FILE:tests_AlibabaCloud_credential>)
endif ()

# <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Install set up >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> #
message(STATUS "${PROJECT_NAME} : Project will be installed to ${CMAKE_INSTALL_PREFIX}")

include(GNUInstallDirs)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})

set(INSTALL_LIBDIR ${CMAKE_INSTALL_LIBDIR} CACHE PATH "Installation directory for libraries")
set(INSTALL_BINDIR ${CMAKE_INSTALL_BINDIR} CACHE PATH "Installation directory for executables")
set(INSTALL_INCLUDEDIR ${CMAKE_INSTALL_INCLUDEDIR} CACHE PATH "Installation directory for header files")
if(WIN32 AND NOT CYGWIN)
    set(DEF_INSTALL_CMAKEDIR CMake)
else()
    set(DEF_INSTALL_CMAKEDIR share/cmake/${PROJECT_NAME})
endif()
set(INSTALL_CMAKEDIR ${DEF_INSTALL_CMAKEDIR} CACHE PATH "Installation directory for CMake files")

foreach(p LIB BIN INCLUDE CMAKE)
    file(TO_NATIVE_PATH ${CMAKE_INSTALL_PREFIX}/${INSTALL_${p}DIR} _path )
    message(STATUS "Installing ${p} components to ${_path}")
    unset(_path)
endforeach()

# Prepare RPATH (Unix-like systems only)
if(UNIX)
    file(RELATIVE_PATH _rel ${CMAKE_INSTALL_PREFIX}/${INSTALL_BINDIR} ${CMAKE_INSTALL_PREFIX})
    if(APPLE)
        set(_rpath "@loader_path/${_rel}")
    else()
        set(_rpath "\$ORIGIN/${_rel}")
    endif()
    file(TO_NATIVE_PATH "${_rpath}/${INSTALL_LIBDIR}" install_RPATH)
    
    set_target_properties(${PROJECT_NAME}
            PROPERTIES
            MACOSX_RPATH ON
            SKIP_BUILD_RPATH OFF
            BUILD_WITH_INSTALL_RPATH OFF
            INSTALL_RPATH "${install_RPATH}"
            INSTALL_RPATH_USE_LINK_PATH ON)
endif()

install(
        TARGETS
        ${PROJECT_NAME}
        EXPORT
        alibabacloud_credentialTargets
        ARCHIVE
        DESTINATION ${INSTALL_LIBDIR}
        COMPONENT lib
        RUNTIME
        DESTINATION ${INSTALL_BINDIR}
        COMPONENT bin
        LIBRARY
        DESTINATION ${INSTALL_LIBDIR}
        COMPONENT lib
        PUBLIC_HEADER
        DESTINATION ${INSTALL_INCLUDEDIR}/AlibabaCloud
        COMPONENT dev
)

# Install cmake config
install(
        EXPORT
        alibabacloud_credentialTargets
        NAMESPACE
        "alibabacloud_credential::"
        DESTINATION
        ${INSTALL_CMAKEDIR}
        COMPONENT
        dev
)
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
)
configure_package_config_file(
        ${PROJECT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        INSTALL_DESTINATION ${INSTALL_CMAKEDIR}
)
install(
        FILES
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake
        DESTINATION
        ${INSTALL_CMAKEDIR}
)