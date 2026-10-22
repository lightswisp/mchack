# mchack
An experimental injectable minecraft 1.16 cheat for linux x86-64. It uses nuklear gui library to draw esp, text and menu.

<img width="2879" height="1738" alt="Screenshot From 2026-10-21 19-58-30" src="https://github.com/user-attachments/assets/f609cfd3-737c-4d63-b8c3-4db8d3133110" />

## Important notice
I wrote this project for myself, and it is not production ready, so please keep your expectations low. It may crash at any moment due to memory bugs. Also, it doesn't support automatic signature deobfuscation for now, so you will have to patch your minecraft jar yourself in order for this cheat to work. 

## Features

### Aimbot
-   Binds
-   Bone selection, smoothing, fov, etc 
-   Triggerbot

### Esp
-   Boxes
-   Glow
-   Snaplines
-   Health bar
-   Arrows

### Misc
-   Fastplace
-   Fly
-   Fullbright
-   Infinite Jump
-   Nofall
-   Speedhack
-   Autosprint

### Scripts
-   TODO

## Dependencies
- base-devel / build-essential
- cmake 
- jdk-openjdk 
- glew 
- glfw 
- mesa

## How to build it? 

Clone it first
```bash
git clone --recurse-submodules https://github.com/lightswisp/mchack.git
cd mchack 
```

Then create build directory and invoke cmake
```bash
mkdir build && cd build
cmake .. 
make
```

## How to use/inject it?
After building the project, you will need to navigate to the "release" folder in the root of the cloned directory. You will find two files there: "injector" and "libcheat.so".
This injector binary is used to inject/uninject the cheat. You will need to launch Minecraft first, and only then invoke the injector with root privileges from this "release" directory.

If you want to uninject it, just press Ctrl+C.

You can also inject it using GDB if my injector doesn't work for some reason. Use this post as a reference: https://magisterquis.github.io/2018/03/11/process-injection-with-gdb.html
