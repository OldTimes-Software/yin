add_executable(tests tests.c)

set_target_properties(tests PROPERTIES FOLDER "Utilities")

# Setup link libraries
if (NOT UNIX AND NOT MSVC)
    target_link_libraries(tests mingw32)
endif ()
target_link_libraries(tests plcore yin-common yin-node)
