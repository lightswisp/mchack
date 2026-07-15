project(cheat)

option(ZYDIS_BUILD_TOOLS "" OFF)
option(ZYDIS_BUILD_EXAMPLES "" OFF)

find_package(JNI REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 3.3 REQUIRED)

add_compile_options(-g)
add_library(cheat SHARED main.c
    core/message.h
    core/message.c
    utils/logger.c
    utils/logger.h
    hooks/hooks.h
    hooks/hooks.c
    core/minecraft.c
    core/minecraft.h
    core/player.h
    core/player.c
    rendering/draw.c
    rendering/draw.h
    jvm.c
    jvm.h
    game.h
    game.c
    rendering/gui.c
    rendering/gui.h)
target_include_directories(cheat PRIVATE ${JNI_INCLUDE_DIRS})

target_link_libraries(cheat PRIVATE OpenGL::GL)
target_link_libraries(cheat PRIVATE OpenGL::GLU)
target_link_libraries(cheat PRIVATE GLEW::GLEW)
target_link_libraries(cheat PRIVATE glfw)
target_link_libraries(cheat PRIVATE "Zydis")