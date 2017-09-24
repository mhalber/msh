@echo off
REM Need to output denug info
REM Need to add variables
REM How to prevent having to run vcvarsall all times
REM @call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
REM pushd examples
REM cl /c /I..\ /I..\extern /I"C:\Program Files\GLFW\include" /I..\extern\nanovg\example /nologo /D WIN32 /D _WINDOWS /D NDEBUG /D _CRT_SECURE_NO_WARNINGS /MD /W3 /fp:precise /Fo"out\\" msh_draw_example.c ..\extern\glad\glad.c
REM link /OUT:"..\bin\msh_draw_example.exe" /NOLOGO /INCREMENTAL:NO "C:\Program Files\GLFW\lib\glfw3.lib" user32.lib gdi32.lib shell32.lib  /SUBSYSTEM:CONSOLE /ENTRY:"mainCRTStartup" out\glad.obj out\msh_draw_example.obj
REM popd
REM add /SUBSYSTEM:WINDOWS to get rid of console prints

REM WITH NANOVG
pushd examples
cl /c /I..\ /I..\extern /I"C:\Program Files\GLFW\include" /I..\extern\nanovg\src /I..\extern\nanovg\example /nologo /D WIN32 /D _WINDOWS /D NDEBUG /D _CRT_SECURE_NO_WARNINGS /MD /W3 /fp:precise /Fo"out\\" msh_draw_example.c ..\extern\glad\glad.c ..\extern\nanovg\src\nanovg.c
link /OUT:"..\bin\msh_draw_example.exe" /NOLOGO /INCREMENTAL:NO "C:\Program Files\GLFW\lib\glfw3.lib" user32.lib gdi32.lib shell32.lib  /SUBSYSTEM:CONSOLE /ENTRY:"mainCRTStartup" out\glad.obj out\msh_draw_example.obj out\nanovg.obj
popd