#
# CMakeLists.txt
#
# Build of the Google Test external project
#
# Modified to always build from source to ensure correct Runtime Library (MD/MT) matching.
#

cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.10.0
)
FetchContent_GetProperties(googletest)

if(NOT googletest_POPULATED)
  FetchContent_Populate(googletest)
  
  # Force GoogleTest to use shared CRT (MD) on Windows
  # This must be set BEFORE add_subdirectory
  if(MSVC)
    # Set global runtime library setting for googletest subdirectory
    # Use CACHE to ensure it propagates to subdirectories
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreadedDLL" CACHE STRING "" FORCE)
    
    # Prevent GoogleTest from overriding our compiler/linker options
    # This is the key setting that googletest checks
    set(gtest_force_shared_crt ON CACHE BOOL "Force GoogleTest to use shared CRT" FORCE)
    
    # Also set as regular variable for immediate use
    set(gtest_force_shared_crt ON)
  endif()
  
  # Prevent GoogleTest from using PThreads
  set(gtest_disable_pthreads ON CACHE BOOL "" FORCE)

  # adds the targers: gtest, gtest_main, gmock, gmock_main
  add_subdirectory(
    ${googletest_SOURCE_DIR}
    ${googletest_BINARY_DIR}
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
        
        # First, get and clean existing compile options to remove any /MT flags
        get_target_property(_existing_opts ${_tgt} COMPILE_OPTIONS)
        if(_existing_opts)
          # Remove /MT and /MTd flags from existing options
          list(FILTER _existing_opts EXCLUDE REGEX "/MT")
          # Remove any existing /MD flags to avoid duplicates
          list(FILTER _existing_opts EXCLUDE REGEX "/MD")
        endif()
        
        # Force /MD flag directly on the target using generator expressions
        # This ensures both Release and Debug builds use the correct runtime
        # Add /MD after cleaning existing options to ensure it takes precedence
        target_compile_options(${_tgt} PRIVATE
          ${_existing_opts}
          "$<$<CONFIG:Release>:/MD>"
          "$<$<CONFIG:MinSizeRel>:/MD>"
          "$<$<CONFIG:RelWithDebInfo>:/MD>"
          "$<$<CONFIG:Debug>:/MDd>")
        
        # Also ensure linker uses the correct runtime
        # Remove any /NODEFAULTLIB flags that might link static runtime
        get_target_property(_link_flags ${_tgt} LINK_FLAGS)
        if(NOT _link_flags)
          set(_link_flags "")
        endif()
        # Remove /NODEFAULTLIB:libcmt and similar static runtime libs
        string(REPLACE "/NODEFAULTLIB:libcmt" "" _link_flags "${_link_flags}")
        string(REPLACE "/NODEFAULTLIB:libcmtd" "" _link_flags "${_link_flags}")
        string(REPLACE "/NODEFAULTLIB:msvcrt" "" _link_flags "${_link_flags}")
        
        # Add /NODEFAULTLIB flags to prevent static runtime linking
        # libcmt = static C runtime, libcpmt = static C++ runtime
        # Use target_link_options for CMake 3.13+, otherwise use LINK_FLAGS
        if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.13")
          target_link_options(${_tgt} PRIVATE
            "$<$<CONFIG:Release>:/NODEFAULTLIB:libcmt>"
            "$<$<CONFIG:Release>:/NODEFAULTLIB:libcpmt>"
            "$<$<CONFIG:MinSizeRel>:/NODEFAULTLIB:libcmt>"
            "$<$<CONFIG:MinSizeRel>:/NODEFAULTLIB:libcpmt>"
            "$<$<CONFIG:RelWithDebInfo>:/NODEFAULTLIB:libcmt>"
            "$<$<CONFIG:RelWithDebInfo>:/NODEFAULTLIB:libcpmt>"
            "$<$<CONFIG:Debug>:/NODEFAULTLIB:libcmtd>"
            "$<$<CONFIG:Debug>:/NODEFAULTLIB:libcpmtd>")
          # Still set LINK_FLAGS for any existing flags
          if(_link_flags)
            set_target_properties(${_tgt} PROPERTIES LINK_FLAGS "${_link_flags}")
          endif()
        else()
          # For older CMake versions, append to LINK_FLAGS
          set_target_properties(${_tgt} PROPERTIES
            LINK_FLAGS "${_link_flags} /NODEFAULTLIB:libcmt$<$<CONFIG:Debug>:d> /NODEFAULTLIB:libcpmt$<$<CONFIG:Debug>:d>")
        endif()
      endif()
    endforeach()
  endif()
endif()
