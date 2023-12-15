@echo off

:loop
REM Clear screen
cls

REM Generate 4 random numbers from 0 to 255
set /a num1=%random% %% 256
set /a num2=%random% %% 256
set /a num3=%random% %% 256
set /a num4=%random% %% 256

REM Print current time
echo Current time: %time%

REM Start the .exe with /IP parameter and the 4 random numbers
"..\Client-exe-x86-64\clgeoip.exe" /IP %num1%.%num2%.%num3%.%num4% /NoLogo /Transport PIPE

REM Wait for 1 second (optional, adjust as needed)
REM ping -n 2 127.0.0.1 > nul

goto loop
