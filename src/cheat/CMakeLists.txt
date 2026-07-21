project(cheat)

option(ZYDIS_BUILD_TOOLS "" OFF)
option(ZYDIS_BUILD_EXAMPLES "" OFF)

find_package(JNI REQUIRED)
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 3.3 REQUIRED)

add_library(cheat SHARED main.c
    core/message.h
    core/message.c
    utils/logger.c
    utils/logger.h
    hooks/hooks.h
    hooks/hooks.c
    core/localplayer.h
    core/localplayer.c
    rendering/draw.c
    rendering/draw.h
    jvm.c
    jvm.h
    game.h
    game.c
    rendering/gui.c
    rendering/gui.h
    hacks/esp/esp.c
    hacks/esp/esp.h
    core/player.c
    core/player.h
    hacks/misc/fly.c
    hacks/misc/sprint.c
    hacks/misc/sprint.h
    hacks/misc/fly.h
    hacks/misc/nofall.c
    hacks/misc/nofall.h
    hacks/misc/misc.c
    hacks/misc/misc.h
    hacks/misc/infjump.c
    hacks/misc/infjump.h
    hacks/aimbot/aimbot.c
    hacks/aimbot/aimbot.h
    utils/math.h
    core/updater.c
    core/updater.h
    hacks/misc/speed.c
    hacks/misc/speed.h)
target_include_directories(cheat PRIVATE ${JNI_INCLUDE_DIRS})
target_compile_options(cheat PRIVATE -g -O0 -fno-omit-frame-pointer)

target_link_libraries(cheat PRIVATE OpenGL::GL)
target_link_libraries(cheat PRIVATE OpenGL::GLU)
target_link_libraries(cheat PRIVATE GLEW::GLEW)
target_link_libraries(cheat PRIVATE glfw)
target_link_libraries(cheat PRIVATE "Zydis")