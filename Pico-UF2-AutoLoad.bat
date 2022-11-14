@echo off
::Fully qualified path of the raspberry pi pico when it appears as a drive to the pc
::This is usually a driver letter on windows, followed by a backslash as shown below
set PI_LOAD_DIR="G:\"

::Fully qualified path of the .uf2 to load
::Below is an example directory, use backslashes
set UF2_FILE="f:\_Backup\Raspberry PICO\Programs\pico-libraries\build\pico-libraries.uf2"

::How long between checks
set /A SLEEP_TIME=1

echo "Raspberry Pi Pico Autoloader By Conor Shore 2021"
echo "Minimise this window, it will sit and wait for a Pico to be plugged in"

:loop


if EXIST %PI_LOAD_DIR% ( 
       
	echo "Pi Pico detected!"
    copy %UF2_FILE% %PI_LOAD_DIR% >>NUL
)
timeout %SLEEP_TIME% >> NUL
goto :loop

done