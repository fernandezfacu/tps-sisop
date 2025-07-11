#!/bin/bash

mkdir prueba
./fisopfs prueba
cd prueba

echo "-----------------------------------------------"
echo "PRUEBAS DE CREACION DE ARCHIVOS"
echo ""
echo "Se ejecutará la siguiente secuencia de comandos:"
echo ""
echo "touch file"
echo "ls"

touch file
echo ""
echo "Salida de ls:"
echo ""
ls

cd ..
sudo umount prueba
rmdir prueba

