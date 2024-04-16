set CFLAGS=/Zi /EHsc /MD /nologo /std:c++20
set LIBS=raylib.lib opengl32.lib winmm.lib gdi32.lib shell32.lib user32.lib
set LFLAGS=/Foobj\\ /Fdbuild\\ 
cl %CFLAGS% %1 %LIBS% %LFLAGS% /Febuild/out.exe
