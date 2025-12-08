#
# CMakeLists.txt
#
# Build of the Google Test external project
#
# Modified to always build from source to ensure correct Runtime Library (MD/MT) matching.
#

cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

# Force GoogleTest to use shared CRT (MD) on Windows
# This must be set BEFORE FetchContent or add_subdirectory
if(MSVC)
  # This is the key setting that googletest checks
  set(gtest_force_shared_crt ON CACHE BOOL "Force GoogleTest to use shared CRT" FORCE)
  
  # Set global runtime library setting
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL" CACHE STRING "MSVC runtime library" FORCE)
  
  # Ensure policy is set for CMake 3.15+
  if(POLICY CMP0091)
    cmake_policy(SET CMP0091 NEW)
  endif()
endif()

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
FetchContent_GetProperties(googletest)

if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  
  # Prevent GoogleTest from using PThreads
  set(gtest_disable_pthreads ON CACHE BOOL "" FORCE)

  # adds the targers: gtest, gtest_main, gmock, gmock_main
  add_subdirectory(
    ${googletest_SOURCE_DIR}
    ${googletest_BINARY_DIR}
    EXCLUDE_FROM_ALL
    )

  # Silence std::tr1 warning on MSVC and ensure runtime library consistency
  if(MSVC)
    foreach(_tgt gtest gtest_main gmock gmock_main)
      if(TARGET ${_tgt})
        target_compile_definitions(${_tgt}
          PRIVATE
            "_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING"
          )
        
        # Explicitly set runtime library to match project settings
        # MSVC_RUNTIME_LIBRARY property requires CMake 3.15+
        if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.15")
          set_target_properties(${_tgt} PROPERTIES
            MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
        endif()
        
        # Force /MD flag directly on the target using generator expressions
        # This ensures both Release and Debug builds use the correct runtime
        target_compile_options(${_tgt} PRIVATE
          "$<$<CONFIG:Release>:/MD>"
          "$<$<CONFIG:MinSizeRel>:/MD>"
          "$<$<CONFIG:RelWithDebInfo>:/MD>"
          "$<$<CONFIG:Debug>:/MDd>")
        
        # Explicitly exclude static C++ runtime libraries
        if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.13")
          target_link_options(${_tgt} PRIVATE
            /NODEFAULTLIB:libcmt
            /NODEFAULTLIB:libcpmt
            "$<$<CONFIG:Debug>:/NODEFAULTLIB:libcmtd>"
            "$<$<CONFIG:Debug>:/NODEFAULTLIB:libcpmtd>")
        else()
          # For older CMake versions, use LINK_FLAGS
          get_target_property(_link_flags ${_tgt} LINK_FLAGS)
          if(NOT _link_flags OR _link_flags STREQUAL "NOTFOUND")
            set(_link_flags "")
          endif()
          set_target_properties(${_tgt} PROPERTIES
            LINK_FLAGS "${_link_flags} /NODEFAULTLIB:libcmt /NODEFAULTLIB:libcpmt")
        endif()
      endif()
    endforeach()
  endif()
endif()
