# Tarea 3 ELO330 2s2022


# Compilación

Primero hay que compilar los archivos fuente, con los siguientes comandos:
### recuento.c
```
gcc recuento.c -o recuento
```
### acumulador.c
```
gcc acumulador.c -o acumulador -lpthread
```
### Visualizador.java
```
javac Visualizador.java
```

# Ejecución
Es necesario ejecutar el servidor primero, para esto usar

```
./acumulador <puerto>
```

Luego, ejecutar `recuento` y `Visualizador`, respectivamente con

```
./recuento <direccion_ip> <puerto>
```

```
java Visualizador <direccion_ip> <puerto + 1>
```

# Ejemplo

Corriendo los tres procesos en Aragorn, con el servidor en el puerto 47200

```
./acumulador 47200
```

```
./recuento localhost 47200
```

```
java Visualizador localhost 47201
```

También es posible usar `aragorn.elo.utfsm.cl` como dirección IP.