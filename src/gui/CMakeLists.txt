set(CMAKE_C_STANDARD 11)

add_library(yin-gui STATIC
        private/gui.c
        private/gui_button.c
        private/gui_cursor.c
        private/gui_draw.c
        private/gui_font_bitmap.c
        private/gui_image.c
        private/gui_panel.c
        )

target_include_directories(yin-gui
        PRIVATE
        private/
        PUBLIC
        public/
        )

set_target_properties(yin-gui PROPERTIES
    FOLDER "Engine"
    PREFIX ""
)

target_link_libraries(yin-gui
        plgraphics
        yin-common
        )
