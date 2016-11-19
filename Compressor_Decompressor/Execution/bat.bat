@echo off
set loopcount=5
:loop 
echo Compression with %loopcount% bits
Compressor.exe %loopcount%
echo Decompression with %loopcount% bits
Decompressor.exe %loopcount%
set /a loopcount=loopcount+1
if %loopcount%==17 goto exitloop
goto loop
:exitloop
pause