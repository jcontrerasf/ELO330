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

Compilar con: gcc -o cshmem cshmem.c -lrt -pthread

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <errno.h>
#include <signal.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>

int contador_bytes = 0;
int contador_bytes_antiguo = 0;
int contador_vueltas = 0;
int *tasa_medida;

void sig_handler(int signum){
    // Guardar tasa medida
    tasa_medida[contador_vueltas] = 10 * (contador_bytes - contador_bytes_antiguo); // guardo bytes por segundo
    // Reiniciar contador_bytes y sumarle uno a contador_vueltas
    contador_bytes_antiguo = contador_bytes;
    contador_vueltas++;
}


int main(int argc, char *argv[])
{
    if(argc!=3) {
        printf("Uso: %s <prefix_name> <N>\n", argv[0]);
        exit(1);
    }

    const int SIZE = 131072;  // file size 128*2^10 128k
    char *prefix = argv[1];
    int N = atoi(argv[2]);

    printf("prefijo: %s, N: %d\n", prefix, N);

    //Memoria compartida
    char name[100]; // file name
    sprintf(name, "/%s_shmem", prefix);

    //Semáforos
    char nameEmpty[100];
    char nameFull[100];
    // Nombre para los semaforos
    sprintf(nameEmpty, "/%s_EMPTY", prefix);
    sprintf(nameFull, "/%s_FULL", prefix);

    
    tasa_medida = (int *)malloc(10*N*sizeof(char));


    struct itimerval timer;
    

    signal(SIGALRM, sig_handler);

    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 100000;

    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 100000;



    int shm_fd;       // file descriptor, from shm_open()
    char *shm_base;   // base address, from mmap()
    char *ptr;        // shm_base is fixed, ptr is movable
    sem_t * empty_sem, *full_sem;

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

    /**
     * Create two named semaphores
     */

    // first remove the semaphore if it already exists
    if (sem_unlink(nameEmpty) == -1)
            printf("Error removing %s: %s\n", nameEmpty, strerror(errno));
    if (sem_unlink(nameFull) == -1)
            printf("Error removing %s: %s\n", nameEmpty, strerror(errno));

    // create and initialize the semaphore
    if ( (empty_sem = sem_open(nameEmpty, O_CREAT, 0666, 1)) == SEM_FAILED)
            printf("Error creating %s: %s\n", nameEmpty, strerror(errno));
    if ( (full_sem = sem_open(nameFull, O_CREAT, 0666, 0)) == SEM_FAILED)
            printf("Error creating %s: %s\n", nameFull, strerror(errno));


    //CONSUMIR DATOS

    char buf[1];
    int sum = 0;
    int n = read(shm_fd, buf, sizeof(buf));

    setitimer(ITIMER_REAL, &timer, NULL);

    do{
        sum += (int)(buf[0]);
        contador_bytes++;
        if((int)(buf[0]) == 0)
            break;
       

    } while (n = read(shm_fd, buf, sizeof(buf)));


    sem_close(empty_sem);
    sem_unlink(nameEmpty);
    sem_close(full_sem);  
    sem_unlink(nameFull);

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

    /* remove the shared memory segment from the file system */
    if (shm_unlink(name) == -1) {
        printf("cons: Error removing %s: %s\n", name, strerror(errno));
        exit(1);
    }

    return 0;
}