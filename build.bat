@echo off

set APP_NAME=raycaster

set SRC_DIR=src\*.c

set COMPILER_OPTS=/Fdpdb\ /Febin\%APP_NAME%.exe /Foobj\ /Iinclude  /Zi
set LINKER_OPTS=/subsystem:console /libpath:lib
set LIBS=SDL2_image.lib SDL2_mixer.lib SDL2_net.lib SDL2_ttf.lib SDL2.lib SDL2main.lib SDL2test.lib

rd bin /s /q
rd obj /s /q 
rd pdb /s /q

md bin
md obj
md pdb

cl %COMPILER_OPTS% %SRC_DIR% /link %LINKER_OPTS% %LIBS%

copy assets\* .\bin
