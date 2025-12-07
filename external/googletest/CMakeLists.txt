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
  # Prevent GoogleTest from overriding our compiler/linker options
  # when building with Visual Studio
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
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
      endif()
    endforeach()
  endif()
endif()
