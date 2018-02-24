@echo off

set APP_NAME=raycaster

set SRC_DIR=src\*.c

set INCLUDE_DIRS=/Iinclude\SDL2-2.0.7 /Iinclude\SDL2_image-2.0
set LIB_DIRS=/libpath:lib\SDL2-2.0.7 /libpath:lib\SDL2_image-2.0
set LIBS=SDL2.lib SDL2main.lib SDL2test.lib SDL2_image.lib

set COMPILER_OPTS=/Fdpdb\ /Febin\%APP_NAME%.exe /Foobj\ /Zi
set LINKER_OPTS=/subsystem:console

rd bin /s /q
rd obj /s /q 
rd pdb /s /q

md bin
md obj
md pdb

cl %INCLUDE_DIRS% %COMPILER_OPTS% %SRC_DIR% /link %LIB_DIRS% %LINKER_OPTS% %LIBS%

copy assets\* .\bin
