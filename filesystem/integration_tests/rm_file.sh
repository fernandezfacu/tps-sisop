#!/bin/bash

mkdir prueba
./fisopfs prueba
cd prueba

echo "-----------------------------------------------"
echo "PRUEBAS DE ELIMINACIÓN DE ARCHIVOS"
echo ""
echo "Se ejecutará la siguiente secuencia de comandos:"
echo ""
echo "touch file1 file2 file3"
echo "ls"
echo "rm file2"
echo "ls"
echo "unlink file1"
echo "ls"

touch file1 file2 file3
echo ""
echo "Salida del primer ls:"
echo ""
ls

rm file2
echo ""
echo "Salida del segundo ls (elimina file2):"
echo ""
ls

unlink file3
echo ""
echo "Salida del tercer ls (elimina file3):"
echo ""
ls

cd ..
sudo umount prueba
rmdir prueba