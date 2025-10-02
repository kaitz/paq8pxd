@echo off
echo Build paq8pxd

set cdir=%CD%
set path=%cdir%\tools\mingw64\bin\..\;%cdir%\tools\mingw64\x86_64-w64-mingw32\lib
set gcc=%cdir%\tools\mingw64\bin\g++.exe 
set src=%cdir%
rem -ffp-model=fast 
set options=-DWINDOWS -DNDEBUG -DMT  -DSM  -m64 -Wall -std=gnu++11 -mavx2 -O3 -fno-threadsafe-statics -Wno-unused-variable  -Wno-unused-but-set-variable -Wno-format -mfpmath=sse -march=corei7 -ffast-math -fno-math-errno -fno-trapping-math 

del _error_log.txt  >nul 2>&1
del paq8pxd.exe      >nul 2>&1

set models_list=
for /f "tokens=*" %%F in ('dir /b /a:-d "%src%\models\*.cpp"') do call set models_list=%%models_list%% "%src%\models\%%F"
set predictors_list=
for /f "tokens=*" %%F in ('dir /b /a:-d "%src%\predictors\*.cpp"') do call set predictors_list=%%predictors_list%% "%src%\predictors\%%F"
set wrt_list=
for /f "tokens=*" %%F in ('dir /b /a:-d "%src%\prt\wrt\*.cpp"') do call set wrt_list=%%wrt_list%% "%src%\prt\wrt\%%F"
set prt_list=
for /f "tokens=*" %%F in ('dir /b /a:-d "%src%\prt\*.cpp"') do call set prt_list=%%prt_list%% "%src%\prt\%%F"
set stream_list=
for /f "tokens=*" %%F in ('dir /b /a:-d "%src%\stream\*.cpp"') do call set stream_list=%%stream_list%% "%src%\stream\%%F"
set filters_list=
for /f "tokens=*" %%F in ('dir /b /a:-d "%src%\filters\*.cpp"') do call set filters_list=%%filters_list%% "%src%\filters\%%F"
set src_list=
for /f "tokens=*" %%F in ('dir /b /a:-d "%src%\*.cpp"') do call set src_list=%%src_list%% "%src%\%%F"
rem pause

IF %ERRORLEVEL% NEQ 0 goto end

%gcc% -static %options% %models_list% %predictors_list% %wrt_list% %prt_list% %stream_list% %filters_list% %src_list% -opaq8pxd.exe -s zlib.a bzip2.a  2>_error_log.txt
IF %ERRORLEVEL% NEQ 0 goto end

:end
del *.o
del "%src%\models\*.o"
del "%src%\predictors\*.o"
del "%src%\prt\*.o"
del "%src%\prt\wrt\*.o"
del "%src%\stream\*.o"
pause
