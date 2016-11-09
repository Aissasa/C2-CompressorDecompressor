@echo off
set loopcount=5
:loop 
echo Compression with %loopcount% bits
start Compressor.exe %loopcount%
pause
echo Decompression with %loopcount% bits
start Decompressor.exe %loopcount%
pause
set /a loopcount=loopcount+1
if %loopcount%==16 goto exitloop
goto loop
:exitloop
pause