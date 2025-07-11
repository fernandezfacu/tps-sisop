#!/bin/bash

mkdir prueba
./fisopfs prueba
cd prueba

echo "-----------------------------------------------"
echo "PRUEBAS DE CREACION DE DIRECTORIOS"
echo ""
echo "Se ejecutará la siguiente secuencia de comandos:"
echo ""
echo "mkdir dir1"
echo "mkdir dir2"
echo "cd dir1"
echo "touch file1 file2 file3"
echo "cd .."
echo "cd dir2"
echo "touch file4"
echo "cd .."
echo "ls"
echo "ls dir1"
echo "rmdir dir1"
echo "ls"

mkdir dir1
mkdir dir2
cd dir1
touch file1 file2 file3
cd ..
cd dir2
touch file4
cd ..

echo ""
echo "Salida de ls (dos directorios en root):"
echo ""
ls

echo ""
echo "Salida de ls dir1 (directorio con tres archivos):"
echo ""
ls dir1

rmdir dir1
echo ""
echo "Salida de ls (un directorio en root tras rmdir dir1):"
echo ""
ls

cd ..
sudo umount prueba
rmdir prueba
