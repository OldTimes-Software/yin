add_library(lisp STATIC
        private/lisp.cpp
        private/lisp_c.cpp
        private/lisp_gc.cpp
        private/lisp_opt.cpp
        )

set_target_properties(lisp PROPERTIES FOLDER "Scripting")

target_include_directories(lisp
        PUBLIC
        public/
        PRIVATE
        private/
        )

target_link_libraries(lisp
        plcore
        )
