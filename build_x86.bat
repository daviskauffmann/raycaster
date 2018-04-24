@echo off

set SRC_DIR=src\*.c

set COMPILER_OPTS=/Fdbuild\x86\ /Febin\x86\raycaster.exe /Fobuild\x86\ /Iinclude /W4 /Zi
set LINKER_OPTS=/libpath:lib\x86\ /subsystem:console
set LIBS=SDL2_image.lib SDL2_mixer.lib SDL2_ttf.lib SDL2.lib SDL2main.lib SDL2test.lib

del bin\x86\*.exe
del bin\x86\*.exp
del bin\x86\*.ilk
del bin\x86\*.lib
del bin\x86\*.pdb

rd build\x86\ /s /q 
md build\x86\

cl %COMPILER_OPTS% %SRC_DIR% /link %LINKER_OPTS% %LIBS%
