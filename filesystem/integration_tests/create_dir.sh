#!/bin/bash

mkdir prueba
./fisopfs prueba
cd prueba

echo "-----------------------------------------------"
echo "PRUEBAS DE CREACION DE DIRECTORIOS"
echo ""
echo "Se ejecutará la siguiente secuencia de comandos:"
echo ""
echo "mkdir dir"
echo "ls"
echo "ls dir"

mkdir dir
echo ""
echo "Salida de ls:"
echo ""
ls

echo ""
echo "Salida de ls dir (directorio nuevo vacio):"
echo ""
ls dir

cd ..
sudo umount prueba
rmdir prueba
