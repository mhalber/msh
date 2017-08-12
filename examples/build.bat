@echo off
REM Need to output denug info
REM Need to add variables
cl /c /I..\ /I..\extern /I"C:\Program Files\GLFW\include" /nologo /D WIN32 /D _WINDOWS /D NDEBUG /D _CRT_SECURE_NO_WARNINGS /MD /W3 /fp:precise /Fo"out\\" msh_draw_example.c ../extern/glad/glad.c 
link /OUT:"..\bin\msh_draw_example.exe" /NOLOGO /INCREMENTAL:NO "E:\programming\libs\glfw-3.2.1\build\src\Release\glfw3.lib" user32.lib gdi32.lib  shell32.lib  /SUBSYSTEM:CONSOLE /ENTRY:"mainCRTStartup" out\glad.obj out\msh_draw_example.obj
REM add /SUBSYSTEM:WINDOWS to get rid of console prints

REM winspool.lib shell32.lib ole32.lib oleaut32.lib uuid.lib comdlg32.lib advapi32.lib