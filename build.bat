@echo off

set EntryFile=../chip8_main.c
set VariableFlags=

set WarningFlags=-nologo -WX -W4 -wd4201 -wd4189 -wd4505 -wd4238 -wd4100 -wd4026

REM -FC = Full path of source file in diagnostics (so emacs can parse errors/warning)
REM -Zi = Creates debug information for Visual Studio debugger (Do I need to turn this off is release builds?)
REM -LD = Build DLL file
REM -Od = Turn off all optimizations
REM -incremental:no = To stop annyoing full link message

set CompilerFlags=%WarningFlags% %VariableFlags% -FC -Zi -Od
set LinkerFlags=-incremental:no -LIBPATH:../chip8/SDL/lib/x64/SDL2.lib
set ExternalLibraries=User32.lib Gdi32.lib Winmm.lib

pushd build

REM cl %CompilerFlags% %EntryFile% %ExternalLibraries% -I SDL/include /link %LinkerFlags%

REM cl %CompilerFlags% ../chip8.c /I W:\chip8\SDL\include\ /link /LIBPATH:W:\chip8\SDL\lib\x64\SDL2.lib /LIBPATH:W:\chip8\SDL\lib\x64\SDL2main.lib /SUBSYSTEM:CONSOLE

cl %CompilerFlags% ..\chip8.c -LD -link -incremental:no /EXPORT:Chip8Cycle

cl %CompilerFlags% ../chip8_sdl.c  /I W:\chip8\SDL\include /link chip8.lib /LIBPATH:W:\chip8\SDL\lib\x64 SDL2.lib SDL2main.lib /SUBSYSTEM:CONSOLE

REM cl %CompilerFlags% ../chip8_assembler.c

popd