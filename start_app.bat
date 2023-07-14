@echo off

C:
cd \Users\luis_
start /min cmd /k "mongod"

D:
cd \Tesis\api-arduino
start /min cmd /c "npm run start"

start /min cmd /c "python3 graficaion.py"

REM Mensaje personalizado antes de la espera
echo Se esta ejecutando la base de datos y el servicio de API para enviar los datos a MONGO.
echo Para cerrar las terminales se presiona cualquier tecla :).

REM Espera a que el usuario presione una tecla para detener el proceso
pause

taskkill /f /im mongod.exe
taskkill /t /f /im cmd.exe