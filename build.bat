@echo off

set CommonCompilerFlags=-Od -Zi -MTd -nologo -WX -W3 -wd4201 -wd4100 -wd4189 -wd4505 -wd4127 -EHsc -MP
set CommonLinkerFlags= -incremental:no -opt:ref user32.lib Comdlg32.lib

rem /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup
set GameLinkerFlags= ..\vendor\raylib\lib\raylibdll.lib 

set ServerFiles=..\server\server_main.cpp

setlocal enabledelayedexpansion
rem Find all shared files
for /r "shared" %%f in (*.cpp) do (
    set "SharedFiles=!SharedFiles! "%%f""
)
rem echo Shared files: !SharedFiles!
rem Find all game files
for /r "game" %%f in (*.cpp) do (
    set "GameFiles=!GameFiles! "%%f""
)
rem echo Game files: !GameFiles!

IF NOT EXIST bin mkdir bin
pushd bin

echo Building game...
cl %CommonCompilerFlags% %GameFiles% %SharedFiles% /link %CommonLinkerFlags% %GameLinkerFlags%

echo Building server...
cl %CommonCompilerFlags% %ServerFiles% %SharedFiles% /link %CommonLinkerFlags%

popd

echo Done.