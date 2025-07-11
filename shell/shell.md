# shell

### Búsqueda en $PATH

#### ¿Cuáles son las diferencias entre la syscall execve(2) y la familia de wrappers proporcionados por la librería estándar de C (libc) exec(3)?

`execve(2)` es una system call nativa del kernel de linux. que recibe por parámetro el pathname de un binario (comando) a ejecutar, un arreglo con el comando y los argumentos, y un arreglo con las variables de entorno. Ejecuta el programa al que refiere el pathname, lo que causa que el programa que está corriendo por el proceso que hace el llamado a la syscall sea reemplazado con un nuevo programa (con nuevo stack y heap inicializados, y segmentos de data inicializada o no). Se mantienen atributos del proceso como los file descriptors, el pid y ppid, entre otras cosas.

A diferencia de `execve(2)`, existe una familia de wrappers que proporciona libc (la librería estándar de C), `exec(3)`. Estas funciones hacen en esencia lo mismo que `execve`, (reemplaza la imagen de un proceso por una nueva) pero son en realidad funciones que hacen un llamado interno a `execve`, proporcionando interfaces/funcionalidades diferentes. 

#### ¿Puede la llamada a exec(3) fallar? ¿Cómo se comporta la implementación de la shell en ese caso?

Sí, la llamada a `exec(3)` puede fallar, por cualquiera de los errores que se especifican en la documentación de `execve(2)`. Estos posibles errores son:

- E2BIG: La cantidad total de bytes en el ambiente (envp) y en la lista de argumentos (argv) es muy grande.
- EACCES: Permisos denegados en el archivo, el scrip o el interpretador de ELF. 
- EAGAIN: Se superaron los recursos de RLIMIT_NPROC.
- EFAULT: El nombre de la ruta del archivo o uno de los punteros a argv or envp se encuentra fuera del espacio accesible.
- EINVAL: En ejecutable de ELF tenía más de un segmento de PT_INTERP.
- EIO:   Error de Entrada/Salida.
- EISDIR: Un interpretador de ELF era un directorio.
- ELIBBAD: Un interpretador de ELF no tenia un formato reconocible.
- ELOOP: El nombre de la ruta al archivo, el script o el interprtador de ELF no fue interpretado correctamente. O se alcansó el máximo de recursividad.
- EMFILE: Se alcanso el limite de pre-procesado en el numero de 'file descriptors'.
- ENAMETOOLONG: Ruta al archivo demaciado larga.
- ENFILE: Se alcanzo el limite del sistema en la cantidad total de archivos abiertos.
- ENOENT: No existe o la ruta al archivo o el script o el interprtador de ELF.
- ENOEXEC: Un ejecutable no tiene formato reconocible, es para la arquitectura incorrecta o tiene algún error que impide su ejecución.
- ENOMEM: Cantidad insuficiente de memoria de kernel.
- ENOTDIR: Un componente de la ruta o el script o el interpretador de ELF no es un directorio.

En estos casos, el valor de retorno es -1 y se setea `errno` para indicar cuál fue el error ocurrido. En nuestra implementación de shell, si se detecta un error al hacer `exec`, se imprime por pantalla comunicando dicho error (`perror(“exec failed”)`) y se sale con -1 (`exit(EXIT_FAILURE)`).

---

### Procesos en segundo plano

#### Detallar cuál es el mecanismo utilizado para implementar procesos en segundo plano.

En *run_cmd* modificamos el código para que suceda lo siguiente: 

Si el comando debe ejecutarse en background, se imprime la info y se hace un wait no bloqueante (utilizando `WNOHANG` en `waitpid(2)`) para poder seguir ejecutando otras líneas. 
Más adelante (para la parte 5 del trabajo), eliminamos el wait no bloqueante ya que la ejecución en segundo plano se maneja de diferente manera.

Si el comando debe ejecutarse en primer plano, se hace un wait bloqueante y luego se muestra la info del comando ejecutado. 
En *exec_cmd* se trata el caso de un comando de tipo BACK, casteando al tipo de dato correspondiente (`struct backcmd`) y se manda a *exec_cmd* el comando a ejecutarse (el de tipo `struct cmd` dentro del otro).

#### ¿Por qué es necesario el uso de señales?

Una señal (signal) es una notificación que se envía a un proceso cuando un determinado evento ocurre. Son análogas a las interrupciones por hardware, ya que interrumpen el flujo normal de la ejecución de un programa. En la mayoría de los casos no es posible predecir la llegada de una señal.

Un proceso puede enviar señales a otro proceso o enviárselas a él mismo. Normalmente la fuente de envío de señales hacia un proceso es el Kernel.

Motivos por los los que el Kernel envía señales a un proceso:

- Una excepción de software.
- El usuario presiona algún caracter especial en la terminal.
- Un evento que tiene que ver con el software ocurrió, como que un proceso hijo terminó (señal `SIGCHLD`).

Una señal es generada por un evento. Una vez generada, es entregada a un determinado proceso, que tomará en algún momento una acción.

El uso de señales es necesario y fundamental para la comunicación entre el Kernel del sistema operativo y procesos, o entre distintos procesos. Permite que un proceso realice acciones por defecto (o determinadas por el programador mediante un handler) ante el arribo de una, evitando así el chequeo constante del cumplimiento de algunas condiciones. Como ejemplos, podemos considerar la interrupción de la ejecución de un programa al intentar dividir por 0 (cuando un proceso intenta hacerlo, recibe la señal `SIGFPE`, excepción de coma flotante) o cuando un usuario presiona CTRL+C (proceso recibe la señal `SIGINT`, que representa una interrupción procedente del teclado).

---

### Flujo estándar

#### Investigar el significado de `2>&1`, explicar cómo funciona su forma general. 
- Mostrar qué sucede con la salida de cat out.txt en el ejemplo. 
- Luego repetirlo, invirtiendo el orden de las redirecciones (es decir, `2>&1` >out.txt). ¿Cambió algo? Compararlo con el comportamiento en bash(1).


`2>&1`: redirige el flujo de salida estándar de error (stderr, file descriptor 2) hacia el mismo destino que la salida estándar (stdout, file descriptor 1). 

`2`: hace referencia al canal "stderr".

`&1`: indica que stderr debe redirigirse al mismo destino que la salida estándar.

```
$ ls -C /home /noexiste >out.txt 2>&1

$ cat out.txt
ls: no se puede acceder a '/noexiste': No existe el archivo o el directorio
/home:
user
```

Ahora, invirtiendo el orden de las redirecciones:

```
$ ls -C /home /noexiste 2>&1 >out.txt

$ cat out.txt
ls: no se puede acceder a '/noexiste': No existe el archivo o el directorio
/home:
user
```

Puede verse que no cambió nada al invertir el orden. En cambio, el comportamiento en bash es el siguiente:

```
$ ls -C /home /noexiste 2>&1 >out.txt
ls: no se puede acceder a '/noexiste': No existe el archivo o el directorio

$ cat out.txt
/home:
user
```

Nuestra implementación de la shell no tiene en cuenta el orden de las redirecciones: siempre redirecciona primero la salida estándar y luego la salida estándar de error. En bash se redirecciona según el orden de los comandos.

---

### Tuberías múltiples

#### Investigar qué ocurre con el exit code reportado por la shell si se ejecuta un pipe
- ¿Cambia en algo?
- ¿Qué ocurre si, en un pipe, alguno de los comandos falla? Mostrar evidencia (e.g. salidas de terminal) de este comportamiento usando bash. Comparar con su implementación.


El exit code será el del proceso que se ejecuta al final del pipe. Puede verse utilizando el comando `echo $?`.

```
$ ls -l | noexiste | wc
noexiste: orden no encontrada
      0       0       0

$ echo $? 
0
```

```
$ ls -l | grep Doc | noexiste
noexiste: orden no encontrada

$ echo $? 
127

```
En nuestra implementación el exit code de un, o múltiples, pipe no depende del exit code de los comandos, sino que es distinto de 0 únicamente cuando falla alguna syscall (`pipe`, `fork` o `dup2`).

Si se quiere ver el exit code de cada comando del pipe, existe la variable de bash `PIPESTATUS`, la cual se utiliza como un vector:

```
$ ls -l | noexiste | wc
noexiste: orden no encontrada
      0       0       0

$ echo "${PIPESTATUS[0]} ${PIPESTATUS[1]} ${PIPESTATUS[2]}"
0 127 0
```

En bash puede setearse `pipefail`, que hace que el valor de retorno de un pipeline sea el valor del último comando (el que esté más a la derecha) que falló, en vez del valor del último comando del pipe.

```
$ set -o pipefail
$ ls -l | noexiste | wc
noexiste: orden no encontrada
      0       0       0

$ echo $?
127
```

---

### Variables de entorno temporarias

#### ¿Por qué es necesario hacerlo luego de la llamada a fork(2)?

Es necesario hacerlo luego de la llamada a `fork` porque las variables de entorno temporales deben ser seteadas en el proceso en el cual se quieren utilizar. Entonces, como en este caso se las quiere utilizar en el comando particular (hijo) y no el de la shell (padre), se las debe setear luego de esto.

#### En algunos de los wrappers de la familia de funciones de exec(3) (las que finalizan con la letra e), se les puede pasar un tercer argumento (o una lista de argumentos dependiendo del caso), con nuevas variables de entorno para la ejecución de ese proceso. Supongamos, entonces, que en vez de utilizar setenv(3) por cada una de las variables, se guardan en un arreglo y se lo coloca en el tercer argumento de una de las funciones de exec(3). 
- ¿El comportamiento resultante es el mismo que en el primer caso? Explicar qué sucede y por qué.
- Describir brevemente (sin implementar) una posible implementación para que el comportamiento sea el mismo.

El comportamiento resultante de usar wrappers de exec a los que se les pasa una lista con las variables de entorno en lugar de utilizar setenv no es el mismo. 

Existe una variable global llamada `environ`, que apunta a un arreglo de punteros a strings, al que se le llama "entorno". Cuando un proceso hace un llamado a `exec`, su nueva imagen utiliza dicho arreglo como su entorno. 

Si un proceso llama a alguno de los wrappers de exec que se les pasa una lista con las variables de entorno, se usa únicamente este arreglo como entorno en lugar de la variable `environ`. 

Si las variables de entorno se setean con `setenv`, estas se añaden a las variables que ya pertenecían al entorno y al hacer `exec`, la nueva imagen del proceso tiene acceso a todas ellas.

Una posible implementación para que el comportamiento sea el mismo es añadir todas las variables de entorno (las preexistentes usando `environ` y las que se quieran agregar) al arreglo que se le pasa por parámetro a los wrappers de exec.

---

### Pseudo-variables

#### Investigar al menos otras tres variables mágicas estándar, y describir su propósito.
- Incluir un ejemplo de su uso en bash (u otra terminal similar).

`$_`: Almacena el último argumento del comando anterior.

```
$ ls -l 

$ echo $_
-l

```
	
`$!`: PID del último proceso ejecutado en segundo plano.

```
$ sleep 3 &
[1] 27470

$ echo $!
27470
[1]+  Hecho                   sleep 3
```

`$$`: PID de la shell actual.

```
$ echo $$
14279

```
---

### Comandos built-in

#### ¿Entre cd y pwd, alguno de los dos se podría implementar sin necesidad de ser built-in? ¿Por qué? ¿Si la respuesta es sí, cuál es el motivo, entonces, de hacerlo como built-in? (para esta última pregunta pensar en los built-in como true y false)

En el caso de `cd` no tendría sentido implementarlo sin que sea un built-in, es decir, creando y ejecutando el comando desde un proceso hijo, ya que lo que se quiere hacer es cambiar el directorio actual del padre, no del hijo y, semánticamente, no tendría sentido hacerlo desde un proceso hijo. 

En el caso de `pwd` se puede implementar de ambos modos porque la impresión del directorio actual se puede hacer desde un proceso hijo sin problema, pero implementándolo como built-in se logra una mejor performance al no tener que crear otro proceso aparte.

---

### Historial

---
