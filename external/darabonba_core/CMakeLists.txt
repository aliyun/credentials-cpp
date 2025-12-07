find_package(darabonba_core QUIET)

if(darabonba_core_FOUND)
  message(STATUS "Found darabonba_core version ${darabonba_core_VERSION}")
else()
  message(STATUS "darabonba_core could not be located, Building darabonba_core instead.")
  include(FetchContent)

  FetchContent_Declare(
    _darabonba_core
    GIT_REPOSITORY https://github.com/CodeSpaceiiii/tea-cpp.git
    GIT_TAG fix-cpp
    CMAKE_ARGS
      -DENABLE_UNIT_TESTS:BOOL="0")

  FetchContent_MakeAvailable(_darabonba_core)
  message(STATUS "Added darabonba_core source : ${_darabonba_core_SOURCE_DIR}")
  message(STATUS "Added darabonba_core binary : ${_darabonba_core_BINARY_DIR}")

  target_include_directories(${PROJECT_NAME} PRIVATE "${_darabonba_core_SOURCE_DIR}/include")
  
  # Make the include directory available to parent scope for tests
  set(DARABONBA_CORE_INCLUDE_DIR "${_darabonba_core_SOURCE_DIR}/include" PARENT_SCOPE)
endif()