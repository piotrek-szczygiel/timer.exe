@echo off

setlocal EnableDelayedExpansion

for /f "usebackq tokens=1,2 delims=,=- " %%i in (`wmic os get LocalDateTime /value`) do @if %%i==LocalDateTime (
    set CURR_DATE_TIME=%%j
)

set exe_name=time.exe

set source_files= ^
    src/main.c

if "%1" == "release" (
    set release_mode=1
) else (
    set release_mode=0
)

set compiler_flags= -nologo -Oi -TP -fp:precise -Gm- -MP -FC -EHsc- -GR- -GF

set compiler_defines= -D_CRT_SECURE_NO_WARNINGS

if %release_mode% EQU 0 ( rem Debug
    set compiler_flags=%compiler_flags% -Od -MDd -Z7
) else ( rem Release
    set compiler_flags=%compiler_flags% -O2 -MT -Z7
    set compiler_defines=%compiler_defines% -DNO_ARRAY_BOUNDS_CHECK
)

set compiler_warnings= ^
    -W4 -WX

set compiler_includes=

set libs= ^
    kernel32.lib

set linker_flags= -incremental:no -opt:ref -subsystem:console

if %release_mode% EQU 0 ( rem Debug
    set linker_flags=%linker_flags% -debug
) else ( rem Release
    set linker_flags=%linker_flags% -debug
)

set compiler_settings=%compiler_includes% %compiler_flags% %compiler_warnings% %compiler_defines%
set linker_settings=%libs% %linker_flags%

del *.pdb > NUL 2> NUL
del *.ilk > NUL 2> NUL

cl %compiler_settings% %source_files% /link %linker_settings% -OUT:%exe_name%

if %errorlevel% neq 0 goto end_of_build

del *.obj > NUL 2> NUL

:end_of_build
