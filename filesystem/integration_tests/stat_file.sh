#!/bin/bash

mkdir prueba
./fisopfs prueba
cd prueba

echo "-----------------------------------------------"
echo "PRUEBAS DE STATS DE ARCHIVOS"
echo ""
echo "Se ejecutará la siguiente secuencia de comandos:"
echo ""
echo "touch file"
echo "stat file"
echo "sleep 1"
echo "echo "Escribo en file" | cat > file"
echo "stat file"
echo "sleep 1"
echo "touch file"
echo "stat file"

touch file
echo ""
echo "Salida del primer stat (archivo nuevo, vacío):"
echo ""
stat file

sleep 1
echo "Escribo en file" | cat > file
echo ""
echo "Salida del segundo stat (cambia size, access y mod):"
echo ""
stat file

sleep 1
touch file
echo ""
echo "Salida del tercer stat (cambia access y mod):"
echo ""
stat file

cd ..
sudo umount prueba
rmdir prueba
