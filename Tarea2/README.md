# Tarea 2 ELO330 2s2022


# Compilación

Primero hay que compilar los archivos fuente, con los siguientes comandos:
### cfifo.c
```
gcc cfifo.c -o cfifo -lrt
```
### pfifo.c
```
gcc pfifo.c -o pfifo -lrt
```
### pshmem.c
```
gcc pshmem.c -o pshmem -lrt -pthread
```
### cshmem.c
```
gcc cshmem.c -o cshmem -lrt -pthread
```
### comparison.c
```
gcc comparison.c -o comparison -lrt
```

# Ejecución
Luego, simplemente ejecutar `comparison` con:

```
./comparison <prefix> <N>
```
donde `prefix` es un prefijo para nombrar los archivos o recursos temporales y `N` es la cantidad de segundos que tarda en realizarse las pruebas.