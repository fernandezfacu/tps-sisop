# fisop-fs

## Documentación de diseño

### Estructuras en memoria

Las siguientes estructuras se encuentran en el archivo `types.h` y almacenan los archivos, directorios y sus metadatos.

#### fs

```c
typedef struct fs {
	inode_t inodes[MAX_INODES];
	u_int8_t inodes_bitmap[MAX_INODES];
	int last_inode;
} fs_t;
```

Tenemos una estructura `fs` que se guardará un arreglo estático de inodos, un bitmap de inodos y un campo `last_inode`, que guardará la posición en el arreglo del último inodo ocupado para evitar recorrerlo innecesariamente cuando se busca por un inodo en particular. Siempre que se crea un archivo o un directorio, se busca el primer inodo libre dentro del arreglo para ocupar (se llama a la función `get_next_free_inode` para esto).

Si se tienen 100 inodos ocupados y se quiere crear un archivo o directorio nuevo, no se podrá y la función correspondiente arrojará `ENOSPC`.

#### inode

```c
typedef struct inode {
	char path[MAX_PATH];
	char data[MAX_CONTENIDO];
	mode_t mode;
	time_t last_mod;
	time_t last_access;
	nlink_t nlink;
	uid_t uid;
	gid_t gid;
	off_t size;
} inode_t;
```

Para representar a los archivos y directorios, tenemos una estructura `inode` que guardará lo siguiente: 

- mode: El modo, que indica el tipo de archivo (en nuestro caso, tenemos el archivo común y el directorio) y algunos permisos que tiene el archivo (ver `stat.h`).

- path: El path absoluto de un archivo o directorio. Cuando se busca un archivo o directorio y se lo quiere modificar de algún modo, se recorre el arreglo de inodos en `fs`, y para los inodos ocupados (según indica el bitmap), se compara el path que se reciben en las funciones fisopfs_* con el guardado (función `fs_get_inode`). El largo máximo de un path está limitado por `MAX_PATH`.

- data: El contenido de un archivo o directorio, es un arreglo de chars y el tamaño máximo es `MAX_CONTENIDO`. En el caso de los directorios, se guarda los paths relativos de los archivos contenidos en ellos, separándolos con un espacio (`' '`).

- size: El tamaño de un archivo o directorio. En el caso de los archivos, se inicializa en 0 y se aumenta y disminuye según lo que se escribe en él. En el caso de los directorios, se inicializa en `MAX_CONTENIDO` y servirá para controlar que no se supere la capacidad máxima al guardar un archivo (un directorio guarda los nombres de los archivos o directorios contenidos en él, pero no puede tener data de tamaño mayor a `MAX_CONTENIDO` al igual que cualquier archivo).

- last_mod y last_access

- nlink

- uid y gid

### Persistencia del filesystem en disco

Al iniciar el sistema de archivos, se ejecuta `fisopfs_init`, función en donde se lee el archivo en donde este guardado el filesystem ("fs.fisopfs" por default, o el nombre que se especifique al lanzar el sistema de archivos). Al tener únicamente una estructura `fs`, simplemente se utiliza una vez la función `fread()`.
El sistema de archivos se mantiene en memoria hasta que es ejecutado `fisopfs_destroy`, que hace que el sistema de archivos persista en disco. Similarmente a init, se escribe con `fwrite()` en el archivo que corresponda (el archivo por default o el que sea indicado).

*Aclaración para el corrector: la persistencia funciona bien al ejecutar en foreground ("./fisopfs -f prueba"), pero no es así cuando se ejecuta en background. Esto fue conversado en clase y se dijo que lo aclaremos aquí.* 

## Tests y pruebas de caja negra

### Tests de implementación interna

Tests implementados para probar el funcionamiento básico de las funciones presentes en `fs.h`. 

Para correrlos, debe compilarse:

```bash
$ make test
```

y luego ejecutar con: 

```bash
$ ./fs_test
```

### Pruebas de comandos en bash

Pruebas para mostrar el funcionamiento del filesystem mediante comandos en un archivo `.sh`. 

Para ejecutar todas juntas: 

```bash
$ chmod +x fisopfs_tests.sh
$ ./fisopfs_tests.sh
```

Si se pretende ejecutar un archivo (*.sh) en particular: 

```bash
$ chmod +x integration_tests/*.sh
$ ./integration_tests/*.sh
```

*Aclaraciones para el corrector:*

1. *Las pruebas funcionan como es esperado si se elimina la función fisopfs_flush, pero al estar implementada falla el filesystem en background y se recibe el error "El otro extremo de la conexión no está conectado". Si se quieren correr las pruebas y que funcionen, eliminar fisopfs_flush. Si se quiere probar el funcionamiento de fisopfs_flush, ejecutar en foreground ("./fisopfs -f prueba").*

2. *No se incluyeron pruebas sobre persistencia por el problema con la persistencia al ejecutar en background, mencionado anteriormente*