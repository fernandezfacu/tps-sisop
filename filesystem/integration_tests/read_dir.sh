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
echo "cd dir"
echo "touch file1 file2"
echo "ls"
echo "echo "Escribo en file1" | cat > file1"
echo "cat file1"
echo "mkdir dir_in_dir"
echo "rm file1"
echo "cd .."
echo "ls dir"

mkdir dir
cd dir
touch file1 file2
echo ""
echo "Salida de ls (desde dir, dos archivos):"
echo ""
ls

echo "Escribo en file1" | cat > file1
echo ""
echo "Salida de cat file1 (archivo en dir):"
echo ""
cat file1

echo ""
echo "Salida mkdir dir_in_dir (no soportado):"
echo ""
mkdir dir_in_dir

rm file1
cd ..
echo ""
echo "Salida de ls dir (desde root, solo queda un archivo en dir):"
echo ""
ls dir

cd ..
sudo umount prueba
rmdir prueba
