set(CMAKE_C_STANDARD 11)

add_library(yin-node STATIC
        private/node.c
        private/node_ds_common.c
        private/node_parser.c
        private/node_preprocessor.c
        )
target_include_directories(yin-node PUBLIC ../node/public)
target_link_libraries(yin-node yin-common plcore plgraphics)
