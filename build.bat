@echo off
shaders\glslc -DVERT -fshader-stage=vert shaders\monke.glsl -o monke.vert.spv
shaders\glslc -DFRAG -fshader-stage=frag shaders\monke.glsl -o monke.frag.spv
clang -fuse-ld=lld window.c -o vktest.exe -Iinclude -lkernel32 -luser32
