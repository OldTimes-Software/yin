add_executable(modelconv
        modelconv.c
        modelconv_loader_md2.c
        modelconv_loader_ocmsh.c
        modelconv_loader_vmdl.c
        )

set_target_properties(modelconv PROPERTIES FOLDER "Utilities")

# Setup link libraries
if (NOT UNIX)
    target_link_libraries(modelconv mingw32)
endif ()
target_link_libraries(modelconv plcore plgraphics plmodel yin-common yin-node)
