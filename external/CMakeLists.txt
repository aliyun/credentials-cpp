message(STATUS "Install libraries")

# Force GoogleTest to use shared CRT on Windows before adding subdirectories
if(MSVC AND ENABLE_UNIT_TESTS)
    set(gtest_force_shared_crt ON CACHE BOOL "Force GoogleTest to use shared CRT" FORCE)
endif()

add_subdirectory(darabonba_core)

if (ENABLE_UNIT_TESTS)
    add_subdirectory(googletest)
endif()
