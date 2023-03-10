cmake_minimum_required(VERSION 3.5.1)

set(CMAKE_C_STANDARD 11)

set(GAME_SHARED_FILES
        private/game.c
        private/game_component_camera.c
        private/game_component_input.c
        private/game_component_mesh.c
        private/game_component_movement.c
        private/game_component_transform.c
        private/game_menu.c
        )
set(GAME_LINK_LIBS
        plcore
        plgraphics
        plmodel
        yin-common
        yin-core
        )
set(GAME_INCLUDE_DIRS
        public/
        )

#############################################
# Target Games

set(GAME_USER_TARGET "feral" CACHE STRING "Game to build.")

set(GAME_TARGET_ONEMANY
        private/onemany/om_game.c)
set(GAME_TARGET_SIM
        private/sim/sim_game.c)
set(GAME_TARGET_FERAL
        private/fw/menu/fw_menu.c
        private/fw/menu/fw_menu_pie.c
        private/fw/fw_building.c
        private/fw/fw_character.c
        private/fw/fw_component_stats.c
        private/fw/fw_game.c
        private/fw/fw_inventory.c
        private/fw/fw_name_generator.c
        private/fw/fw_terrain.c
        private/fw/fw_weapon.c)

#todo: need a better way of setting targets...
if (${GAME_USER_TARGET} STREQUAL "sim")
    set(GAME_TARGET ${GAME_TARGET_SIM})
elseif (${GAME_USER_TARGET} STREQUAL "feral")
    set(GAME_TARGET ${GAME_TARGET_FERAL})
else ()
    message("No valid game target specified!")
endif ()

#############################################

add_library(yin-game STATIC
        ${GAME_SHARED_FILES}
        ${GAME_TARGET}
        )

set_target_properties(yin-game PROPERTIES PREFIX "")

# Setup link libraries
if (NOT UNIX AND NOT MSVC)
    target_link_libraries(yin-game mingw32)
endif ()

target_link_libraries(yin-game ${GAME_LINK_LIBS})
target_include_directories(yin-game PUBLIC public/)
