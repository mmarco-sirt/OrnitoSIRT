@echo off
setlocal
chcp 65001 >nul
title OrnitoSIRT - cargar firmware
cd /d "%~dp0"

set "BIN=OrnitoSIRT_v1.0.bin"
if not exist "%BIN%" (
  echo No encuentro %BIN% en esta carpeta.
  pause
  exit /b 1
)

rem Usa el esptool que instala el Arduino IDE con el soporte "esp32 by Espressif"
set "ESPTOOL="
for /d %%D in ("%LOCALAPPDATA%\Arduino15\packages\esp32\tools\esptool_py\*") do (
  if exist "%%D\esptool.exe" set "ESPTOOL=%%D\esptool.exe"
)
if not defined ESPTOOL (
  where esptool >nul 2>nul && set "ESPTOOL=esptool"
)
if not defined ESPTOOL (
  echo.
  echo  No encuentro esptool. Dos opciones:
  echo   1. Instala Arduino IDE y el soporte "esp32 by Espressif" ^(ver LEEME.md^)
  echo   2. Usa el cargador web desde Chrome o Edge: https://espressif.github.io/esptool-js/
  echo      ^(Connect, direccion 0x0, elige %BIN% y pulsa Program^)
  echo.
  pause
  exit /b 1
)

echo.
echo   OrnitoSIRT · carga de firmware
echo   ==============================
echo   Puertos serie detectados:
powershell -NoProfile -Command "Get-CimInstance Win32_PnPEntity | Where-Object { $_.Name -match '\(COM\d+\)' } | ForEach-Object { '     ' + $_.Name }"
echo.

set "PUERTO=%~1"
if not defined PUERTO set /p "PUERTO=  Escribe el puerto del cubo (por ejemplo COM3): "
if not defined PUERTO exit /b 1

echo.
"%ESPTOOL%" --chip esp32c3 --port %PUERTO% --baud 921600 write_flash 0x0 "%BIN%"
if errorlevel 1 (
  echo.
  echo  No se ha podido cargar. Prueba esto:
  echo   - Usa un cable USB-C de DATOS ^(muchos solo cargan^).
  echo   - Modo descarga: manten BOOT, pulsa y suelta RESET, suelta BOOT y vuelve a ejecutar.
  echo   - Cierra el Arduino IDE ^(Monitor Serie^) si tiene el puerto abierto.
  echo.
  pause
  exit /b 1
)

echo.
echo  ¡Listo! Desenchufa y vuelve a enchufar el cubo si no arranca solo.
echo  Despues conectate al WiFi "OrnitoSIRT" ^(clave: ornitosirt^) o escanea el QR.
echo.
pause
