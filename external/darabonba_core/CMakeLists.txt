find_package(darabonba_core QUIET)
find_package(nlohmann_json REQUIRED)
find_package(CURL REQUIRED)

if(darabonba_core_FOUND)
  message(STATUS "Found darabonba_core version ${darabonba_core_VERSION}")
else()
  message(STATUS "darabonba_core could not be located, Building darabonba_core instead.")
  include(FetchContent)

  FetchContent_Declare(
    _darabonba_core
    # 这段逻辑将从git上下载darabonba_core的源代码
    # GIT_REPOSITORY https://github.com/peze/tea-cpp.git
    # GIT_TAG v2.x
    SOURCE_DIR /Users/wenping/Desktop/tea-cpp
    CMAKE_ARGS
      -DENABLE_UNIT_TESTS:BOOL="0"
      )
  FetchContent_GetProperties(_darabonba_core)
  if(NOT _darabonba_core_POPULATED)
    FetchContent_Populate(_darabonba_core)
    message(STATUS "Added darabonba_core source : ${_darabonba_core_SOURCE_DIR}")
    message(STATUS "Added darabonba_core binary : ${_darabonba_core_BINARY_DIR}")
    add_subdirectory(
      ${_darabonba_core_SOURCE_DIR}
      ${_darabonba_core_BINARY_DIR}
    )
  endif()
  message(STATUS "Added darabonba_core source include : ${_darabonba_core_SOURCE_DIR}/include")
  target_include_directories(${PROJECT_NAME} PUBLIC "${_darabonba_core_SOURCE_DIR}/include")
  # if(ENABLE_UNIT_TESTS)
  #   target_include_directories(tests_AlibabaCloud_credential PRIVATE "${_darabonba_core_SOURCE_DIR}/include")
  # endif()
endif()
