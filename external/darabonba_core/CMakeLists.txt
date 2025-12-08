find_package(darabonba_core QUIET)

if(darabonba_core_FOUND)
  message(STATUS "Found darabonba_core version ${darabonba_core_VERSION}")
else()
  message(STATUS "darabonba_core could not be located, Building darabonba_core instead.")
  include(FetchContent)

  # Prepare CMAKE_ARGS for darabonba_core
  set(_darabonba_core_CMAKE_ARGS
    -DENABLE_UNIT_TESTS:BOOL="0")
  
  # Ensure darabonba_core uses dynamic runtime library on Windows
  if(MSVC)
    # Pass runtime library setting via CMAKE_ARGS if CMake version supports it
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.15")
      list(APPEND _darabonba_core_CMAKE_ARGS
        -DCMAKE_MSVC_RUNTIME_LIBRARY:STRING="MultiThreadedDLL")
    endif()
    # Also pass compiler flags to force /MD
    list(APPEND _darabonba_core_CMAKE_ARGS
      -DCMAKE_CXX_FLAGS_RELEASE:STRING="/MD"
      -DCMAKE_CXX_FLAGS_DEBUG:STRING="/MDd"
      -DCMAKE_C_FLAGS_RELEASE:STRING="/MD"
      -DCMAKE_C_FLAGS_DEBUG:STRING="/MDd")
  endif()

  FetchContent_Declare(
    _darabonba_core
    GIT_REPOSITORY https://github.com/aliyun/tea-cpp.git
    GIT_TAG master
    CMAKE_ARGS ${_darabonba_core_CMAKE_ARGS})

  set(ENABLE_UNIT_TESTS OFF)
  FetchContent_MakeAvailable(_darabonba_core)
  
  # Try to set runtime library on darabonba_core target if it exists
  if(MSVC AND TARGET darabonba_core)
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.15")
      set_target_properties(darabonba_core PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    endif()
    
    # Force /MD flag on darabonba_core to ensure it uses dynamic runtime
    # Add /MD flags using generator expressions
    target_compile_options(darabonba_core PRIVATE
      "$<$<CONFIG:Release>:/MD>"
      "$<$<CONFIG:MinSizeRel>:/MD>"
      "$<$<CONFIG:RelWithDebInfo>:/MD>"
      "$<$<CONFIG:Debug>:/MDd>")
    
    # Explicitly exclude static C++ runtime libraries to prevent conflicts
    # But ensure dynamic C++ runtime (msvcprt.lib) is linked
    if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.13")
      target_link_options(darabonba_core PRIVATE
        "$<$<CONFIG:Release>:/NODEFAULTLIB:libcpmt>"
        "$<$<CONFIG:MinSizeRel>:/NODEFAULTLIB:libcpmt>"
        "$<$<CONFIG:RelWithDebInfo>:/NODEFAULTLIB:libcpmt>"
        "$<$<CONFIG:Debug>:/NODEFAULTLIB:libcpmtd>")
    else()
      # For older CMake versions, use LINK_FLAGS
      get_target_property(_current_link_flags darabonba_core LINK_FLAGS)
      if(NOT _current_link_flags OR _current_link_flags STREQUAL "NOTFOUND")
        set(_current_link_flags "")
      endif()
      set_target_properties(darabonba_core PROPERTIES
        LINK_FLAGS "${_current_link_flags} /NODEFAULTLIB:libcpmt$<$<CONFIG:Debug>:d>")
    endif()
    
    # Note: msvcprt.lib should be automatically linked when using /MD
    # If it's not being linked, the issue is likely that darabonba_core
    # was built with /MT. The compile options above should fix this.
  endif()

  if(UNIX AND NOT APPLE)
      # Set CMP0079 to NEW to allow linking libraries to targets defined in other directories
      cmake_policy(SET CMP0079 NEW)
      
      find_package(PkgConfig QUIET)
      if(PkgConfig_FOUND)
          pkg_check_modules(UUID uuid)
          if(UUID_FOUND AND TARGET darabonba_core)
              # Use plain signature to match upstream usage
              target_link_libraries(darabonba_core ${UUID_LIBRARIES})
              target_include_directories(darabonba_core PUBLIC ${UUID_INCLUDE_DIRS})
          else()
               # Fallback if PkgConfig fails to find uuid
               if(TARGET darabonba_core)
                   target_link_libraries(darabonba_core uuid)
               endif()
          endif()
      else()
           if(TARGET darabonba_core)
               target_link_libraries(darabonba_core uuid)
           endif()
      endif()
  endif()

  message(STATUS "Added darabonba_core source : ${_darabonba_core_SOURCE_DIR}")
  message(STATUS "Added darabonba_core binary : ${_darabonba_core_BINARY_DIR}")

  target_include_directories(${PROJECT_NAME} PRIVATE "${_darabonba_core_SOURCE_DIR}/include")
  
  # Make the include directory available to parent scope for tests
  set(DARABONBA_CORE_INCLUDE_DIR "${_darabonba_core_SOURCE_DIR}/include" PARENT_SCOPE)
endif()