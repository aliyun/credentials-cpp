message(STATUS "Install libraries")
add_subdirectory(darabonba_core)

if (ENABLE_UNIT_TESTS)
    add_subdirectory(googletest)
endif()
