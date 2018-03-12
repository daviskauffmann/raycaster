@echo off

set SRC_DIR=src\client\*.c

set COMPILER_OPTS=/Fdbuild\x86\client\ /Febin\x86\client\client.exe /Fobuild\x86\client\ /Iinclude /W4 /Zi
set LINKER_OPTS=/libpath:lib\x86\ /subsystem:console
set LIBS=SDL2_image.lib SDL2_mixer.lib SDL2_net.lib SDL2_ttf.lib SDL2.lib SDL2main.lib SDL2test.lib

del bin\x86\client\*.exe
del bin\x86\client\*.exp
del bin\x86\client\*.ilk
del bin\x86\client\*.lib
del bin\x86\client\*.pdb

rd build\x86\client\ /s /q 
md build\x86\client\

cl %COMPILER_OPTS% %SRC_DIR% /link %LINKER_OPTS% %LIBS%
