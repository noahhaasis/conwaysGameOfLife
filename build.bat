@echo off
pushd build
cl /Z7 /MT -nologo /W4 ..\conways_game_of_life.c ..\board.c /I "c:\development\sdl2\include" -link /subsystem:console  /DYNAMICBASE "SDL2.lib" "SDL2main.lib"  /LIBPATH:"C:\Development\SDL2\lib\x86"
popd build

