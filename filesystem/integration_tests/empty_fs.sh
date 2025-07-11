#!/bin/bash

mkdir prueba
./fisopfs prueba
cd prueba

echo "-----------------------------------------------"
echo "PRUEBAS SOBRE FILESYSTEM VACIO"
echo ""
echo "Se ejecutará la siguiente secuencia de comandos:"
echo ""
echo "ls"
echo "ls -al"

echo ""
echo "Salida de ls:"
echo ""
ls

echo ""
echo "Salida de ls -al:"
echo ""
ls -al

cd ..
sudo umount prueba 
rmdir prueba