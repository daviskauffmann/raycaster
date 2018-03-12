@echo off

set SRC_DIR=src\server\*.c

set COMPILER_OPTS=/Fdbuild\x86\server\ /Febin\x86\server\server.exe /Fobuild\x86\server\ /Iinclude /W4 /Zi
set LINKER_OPTS=/libpath:lib\x86\ /subsystem:console
set LIBS=SDL2_image.lib SDL2_mixer.lib SDL2_net.lib SDL2_ttf.lib SDL2.lib SDL2main.lib SDL2test.lib

del bin\x86\server\*.exe
del bin\x86\server\*.exp
del bin\x86\server\*.ilk
del bin\x86\server\*.lib
del bin\x86\server\*.pdb

rd build\x86\server\ /s /q 
md build\x86\server\

cl %COMPILER_OPTS% %SRC_DIR% /link %LINKER_OPTS% %LIBS%
