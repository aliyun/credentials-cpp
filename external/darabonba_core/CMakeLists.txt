cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

include(FetchContent)

# Try find_package first, then download from Git
# Pass user's CMAKE_PREFIX_PATH to help find user-specified libraries
if(CMAKE_PREFIX_PATH)
  set(CMAKE_PREFIX_PATH_BACKUP ${CMAKE_PREFIX_PATH})
  list(APPEND CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH})
endif()

find_package(darabonba_core QUIET)

if(darabonba_core_FOUND)
  message(STATUS "Found darabonba_core version ${darabonba_core_VERSION}")
  set(DARABONBA_CORE_FOUND TRUE PARENT_SCOPE)
  return()
else()
  message(STATUS "darabonba_core not found, building from source")
  
  # Prepare CMAKE_ARGS for darabonba_core - Pass all important variables
  set(_darabonba_core_CMAKE_ARGS
    -DENABLE_UNIT_TESTS:BOOL=OFF
    -DBUILD_SHARED_LIBS:BOOL=${BUILD_SHARED_LIBS})
  
  # Pass user's CMAKE_PREFIX_PATH to darabonba_core
  if(CMAKE_PREFIX_PATH)
    string(REPLACE ";" "\\;" CMAKE_PREFIX_PATH_ESCAPED "${CMAKE_PREFIX_PATH}")
    list(APPEND _darabonba_core_CMAKE_ARGS
      -DCMAKE_PREFIX_PATH:STRING=${CMAKE_PREFIX_PATH_ESCAPED})
  endif()
  
  # Pass static library preferences
  if(NOT BUILD_SHARED_LIBS)
    list(APPEND _darabonba_core_CMAKE_ARGS
      -DOPENSSL_USE_STATIC_LIBS:BOOL=ON
      -DCURL_STATICLIB:BOOL=ON)
  endif()
  
  # Pass build type
  if(CMAKE_BUILD_TYPE)
    list(APPEND _darabonba_core_CMAKE_ARGS
      -DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE})
  endif()
  
  # Pass toolchain file if specified
  if(CMAKE_TOOLCHAIN_FILE)
    list(APPEND _darabonba_core_CMAKE_ARGS
      -DCMAKE_TOOLCHAIN_FILE:FILEPATH=${CMAKE_TOOLCHAIN_FILE})
  endif()
  
  # Ensure darabonba_core uses dynamic runtime library on Windows
  if(MSVC)
    list(APPEND _darabonba_core_CMAKE_ARGS
      -DCMAKE_CXX_FLAGS_RELEASE:STRING="/MD"
      -DCMAKE_CXX_FLAGS_DEBUG:STRING="/MDd"
      -DCMAKE_C_FLAGS_RELEASE:STRING="/MD"
      -DCMAKE_C_FLAGS_DEBUG:STRING="/MDd")
  endif()

  # Download from Git
  FetchContent_Declare(
    _darabonba_core
    GIT_REPOSITORY https://github.com/aliyun/tea-cpp.git
    GIT_TAG master
    CMAKE_ARGS ${_darabonba_core_CMAKE_ARGS}
  )
  
  message(STATUS "Configuring darabonba_core with args: ${_darabonba_core_CMAKE_ARGS}")

  option(ENABLE_UNIT_TESTS "Enable unit tests" OFF)
  FetchContent_MakeAvailable(_darabonba_core)
  
  # Configure runtime library on darabonba_core target
  if(MSVC AND TARGET darabonba_core)
    # Force /MD flag on darabonba_core to ensure it uses dynamic runtime
    target_compile_options(darabonba_core PRIVATE
      "$<$<CONFIG:Release>:/MD>"
      "$<$<CONFIG:MinSizeRel>:/MD>"
      "$<$<CONFIG:RelWithDebInfo>:/MD>"
      "$<$<CONFIG:Debug>:/MDd>")
    
    # Explicitly exclude static C++ runtime libraries (CMake 3.13+ required)
    target_link_options(darabonba_core PRIVATE
      "$<$<CONFIG:Release>:/NODEFAULTLIB:libcpmt>"
      "$<$<CONFIG:MinSizeRel>:/NODEFAULTLIB:libcpmt>"
      "$<$<CONFIG:RelWithDebInfo>:/NODEFAULTLIB:libcpmt>"
      "$<$<CONFIG:Debug>:/NODEFAULTLIB:libcpmtd>")
  endif()

  # Linux-specific: link uuid library
  if(UNIX AND NOT APPLE)
    cmake_policy(SET CMP0079 NEW)
    
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
      pkg_check_modules(UUID uuid)
      if(UUID_FOUND AND TARGET darabonba_core)
        target_link_libraries(darabonba_core PRIVATE ${UUID_LIBRARIES})
        target_include_directories(darabonba_core PRIVATE ${UUID_INCLUDE_DIRS})
      else()
        if(TARGET darabonba_core)
          target_link_libraries(darabonba_core PRIVATE uuid)
        endif()
      endif()
    else()
      if(TARGET darabonba_core)
        target_link_libraries(darabonba_core PRIVATE uuid)
      endif()
    endif()
  endif()

  message(STATUS "darabonba_core source: ${_darabonba_core_SOURCE_DIR}")
  message(STATUS "darabonba_core binary: ${_darabonba_core_BINARY_DIR}")

  
  # Make the include directory available to parent scope for tests
  set(DARABONBA_CORE_INCLUDE_DIR "${_darabonba_core_SOURCE_DIR}/include" PARENT_SCOPE)
endif()