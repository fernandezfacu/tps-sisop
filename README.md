# Sistemas Operativos

Trabajos Prácticos para la materia Sistemas Operativos de la Facultad de Ingeniería de la UBA

## Trabajo Individual - lab fork

Lab con el objetivo de familiarizarse con las llamadas al sistema fork(2) (que crea una copia del proceso actual) y pipe(2) (que proporciona un mecanismo de comunicación unidireccional entre dos procesos).

[Consigna](https://fisop.github.io/website/labs/fork/)

## Trabajos Prácticos Grupales

Grupo 12. Integrantes: [Anita Vernieri](https://github.com/avernieri), Facundo Fernández, [Adriana Iglesias](https://github.com/AIglesiasT)

### Shell

Desarrollo de la funcionalidad mínima que caracteriza a un intérprete de comandos shell. Se soporta:
- Ejecución de binarios (con búsqueda en $PATH, con y sin argumentos).
- Tres formas de redirección (>, <, 2> y 2>&1).
- Pipes (|) entre comandos y múltiples pipes anidados.
- Expansión de variables de entorno al ejecutar un comando, variables de entorno temporales y la pseudo-variable $?.
- Comandos built-in (cd, exit, pwd).
- Manejo de procesos en segundo plano.

[Consigna](https://fisop.github.io/website/tps/shell/)

Nota: 10

### Scheduling

Se implementó el mecanismo de cambio de contexto para procesos y el scheduler (i.e. planificador) sobre un sistema operativo preexistente. El kernel a utilizar es una modificación de JOS, un exokernel educativo con licencia libre del grupo de Sistemas Operativos Distribuidos del MIT. Además, se implementa un scheduler con política round robin y otro con política basada en prioridades.

[Consigna](https://fisop.github.io/website/tps/sched/)

Nota: 10

### Filesystem FUSE

Se implementó nuestro propio sistema de archivos (o filesystem) para Linux. El sistema de archivos utiliza el mecanismo de FUSE (Filesystem in USErspace) provisto por el kernel, que nos permitirá definir en modo usuario la implementación de un filesystem. Gracias a ello, el mismo tiene interfaz VFS y puede ser accedido con las syscalls y programas habituales (read, open, ls, etc).

La implementación del filesystem es enteramente en memoria: tanto archivos como directorios son representados mediante estructuras que viven en memoria RAM. Por esta razón, buscamos un sistema de archivos que apunte a la velocidad de acceso, y no al volumen de datos o a la persistencia (algo similar a tmpfs). Aún así, los datos de nuestro filesystem están representados en disco por un archivo.

[Consigna](https://fisop.github.io/website/tps/filesystem/)

Nota: 10
