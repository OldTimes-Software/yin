set(ENGINE_SOURCE_EDITOR
        private/editor/editor.c
        private/editor/editor_commands.c
        private/editor/editor_material_selector.c)
set(ENGINE_SOURCE_CLIENT
        private/client/client.c
        private/client/client_console.c
        private/client/client_input.c
        private/client/client_gui.c

        private/client/audio/audio.c
        private/client/audio/audio_openal.c
        private/client/audio/audio_wav.c
        private/client/audio/audio_xaudio2.cpp
        private/client/audio/micromod.c
        private/client/audio/sdlplayer.c

        private/client/renderer/postprocessing/bloom.c
        private/client/renderer/postprocessing/fxaa.c
        private/client/renderer/postprocessing/postprocessing.c
        private/client/renderer/camera.c
        private/client/renderer/flare.c
        private/client/renderer/font.c
        private/client/renderer/particle.c
        private/client/renderer/renderer.c
        private/client/renderer/renderer_draw.c
        private/client/renderer/renderer_material.c
        private/client/renderer/renderer_rendertarget.c
        private/client/renderer/renderer_texture.c
        private/client/renderer/renderer_viewport.c
        private/client/renderer/renderer_world.c
        private/client/renderer/scenegraph.c
        private/client/renderer/shaders.c
        private/client/renderer/sprite.c
        private/client/renderer/visibility.c
        )

add_library(yin-engine STATIC
        private/console.c

        # Entity System
        private/entity/entity.c
        private/entity/entity_component.c
        private/entity/entity_manager.c

        # Networking
        private/net/net.c
        private/net/net_var.c

        private/filesystem.c
        private/game_interface.c
        private/image.c
        private/memory_manager.c
        private/model.c
        private/profiler.c
        private/scheduler.c
        private/serializer.c
        private/world.c
        private/world_deserialiser.c
        private/world_mesh.c
        private/world_serialiser.c
        private/yin.c

        ${ENGINE_SOURCE_EDITOR}
        ${ENGINE_SOURCE_CLIENT}

        private/legacy/actor.c
        private/legacy/actors/actor_player.c
        private/legacy/actors/sg_actors.c

        private/scripting/lisp_interface.c
        private/server/server.c
        private/vm/vm.c
        )
add_dependencies(yin-engine
        plcore
        plgraphics
        plmodel
        pl_gl_driver
        )

target_include_directories(yin-engine PRIVATE
        private/
        )
target_include_directories(yin-engine PUBLIC
        public/
        )

set_target_properties(yin-engine PROPERTIES
        FOLDER "Engine"
        PREFIX ""
        )

# Setup link libraries
if (NOT UNIX AND NOT MSVC)
    target_link_libraries(yin-engine
            mingw32
            )
endif ()
if (MSVC)
    target_link_libraries(yin-engine
            XAudio2
            )
endif ()

target_link_libraries(yin-engine
        OpenAL
        SDL2
        yin-common
        yin-node
        yin-gui
        yin-game
        plcore
        plgraphics
        plmodel
        lisp
        )
