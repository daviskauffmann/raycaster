@echo off

set SRC_DIR=src\*.c

set COMPILER_OPTS=/Fdbuild\ /Febin\raycaster.exe /Fobuild\ /Iinclude /W4  /Zi
set LINKER_OPTS=/subsystem:console /libpath:lib
set LIBS=SDL2_image.lib SDL2_mixer.lib SDL2_net.lib SDL2_ttf.lib SDL2.lib SDL2main.lib SDL2test.lib

del bin\raycaster.exe
del bin\raycaster.exp
del bin\raycaster.ilk
del bin\raycaster.lib
del bin\raycaster.pdb

rd build /s /q 
md build

cl %COMPILER_OPTS% %SRC_DIR% /link %LINKER_OPTS% %LIBS%
