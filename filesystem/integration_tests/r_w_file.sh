#!/bin/bash

mkdir prueba
./fisopfs prueba
cd prueba

echo "-----------------------------------------------"
echo "PRUEBAS DE LECTURA/ESCRITURA ARCHIVOS"
echo ""
echo "Se ejecutará la siguiente secuencia de comandos:"
echo ""
echo "touch file"
echo "cat file"
echo "echo "Contenido en file" | cat > file"
echo "cat file"
echo "echo "Sobreescribo muchas mas cosas en file" | cat > file"
echo "cat file"
echo "echo "Sobreescribo menos cosas" | cat > file"
echo "cat file"

touch file
echo ""
echo "Salida del primer cat (archivo vacío):"
echo ""
cat file

echo "Contenido en file" | cat > file
echo ""
echo "Salida del segundo cat:"
echo ""
cat file

echo "Sobreescribo muchas mas cosas en file" | cat > file
echo ""
echo "Salida del tercer cat:"
echo ""
cat file

echo "Sobreescribo menos cosas" | cat > file
echo ""
echo "Salida del cuarto cat:"
echo ""
cat file

cd ..
sudo umount prueba
rmdir prueba
