/*

cshmem: consume datos desde memoria compartida

Sintaxis: cshmem <prefix_name> <N>

Descripción: cshmemcrea la zona de memoria compartida y la accede para su lectura.
Desde que lee el primer dato calcula la suma de los valores leídosy cada 100 [ms] mide la tasa
de bytes leídosen los últimos 100 [ms]. cshmemalmacena las tasas medidas en un arreglo de tamaño al menos 10*N,
donde N es el número de segundos en que se correel pshmem. Cuando cshmem lee un 0 en la primera ubicación de la 
memoria, envía a la salida estándar la suma de los valores y las tasas medidas. Previo a terminar debe liberar
todos los recursos del sistema (memoria y semáforos).cshmem usa prefix_name para crearrecursos de memoria
compartida y semáforos necesarios para manejar el traspaso de datos.

Compilar con: gcc -o cshmem cshmem.c -lrt

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <errno.h>


int main(int argc, char *argv[])
{
    if(argc!=3) {
        printf("Uso: cshmem <prefix_name> <N>\n");
        exit(1);
    }

    const char *name = "/shm-elo330";  // file name
    const int SIZE = 4096;      // file size
    char *prefix = argv[1];
    int N = atoi(argv[2]);

    printf("prefijo: %s, N: %d\n", prefix, N);



    int shm_fd;       // file descriptor, from shm_open()
    char *shm_base;   // base address, from mmap()
    char *ptr;        // shm_base is fixed, ptr is movable

    /* create the shared memory segment as if it was a file */
    shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        printf("prod: Shared memory failed: %s\n", strerror(errno));
        exit(1);
    }

    /* configure the size of the shared memory segment */
    ftruncate(shm_fd, SIZE);

    /* map the shared memory segment to the address space of the process */
    shm_base = mmap(0, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_base == MAP_FAILED) {
        printf("prod: Map failed: %s\n", strerror(errno));
        // close and shm_unlink?
        exit(1);
    }



    /* remove the mapped memory segment from the address space of the process */
    if (munmap(shm_base, SIZE) == -1) {
        printf("prod: Unmap failed: %s\n", strerror(errno));
        exit(1);
  }

    /* close the shared memory segment as if it was a file */
    if (close(shm_fd) == -1) {
        printf("prod: Close failed: %s\n", strerror(errno));
        exit(1);
    }

    return 0;


}