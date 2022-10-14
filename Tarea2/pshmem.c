/*

cshmem: consume datos desde memoria compartida

Sintaxis: pshmem <prefix_name> <N>

Descripción: pshmem accede a la zona de memoria compartida previamente creada
por cshmem y por N segundos ejecuta un loop 
Al cabo de N segundos y en respuesta a una alarma, una función modifica el valor de done de “falso” (0)
a “verdadero”(1) y el programa escribe un 0 en la primera posición de la memoria compartiday  termina.
pshmem usa prefix_name para acceder a los recursos de memoria compartida y semáforos necesarios para
manejar el traspaso de datos.

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

int done = 0;

// Funcion para medir ms
long long current_timestamp() {
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds
    return milliseconds;
}

int h;
int *data;


int main(int argc, char *argv[])
{
    clock_t before = clock();
    int nc;
    if(argc!=3) {
        printf("Uso: %s <prefix_name> <N>\n", argv[0]);
        exit(1);
    }

    const int SIZE = 2048; 
    char *prefix = argv[1];
    int N = atoi(argv[2]);

    //Memoria compartida
    char name[100]; // file name
    sprintf(name, "/%s_shmem", prefix);

    //Semáforos
    char nameEmpty[100];
    char nameFull[100];
    // Nombre para los semaforos
    sprintf(nameEmpty, "/%s_EMPTY", prefix);
    sprintf(nameFull, "/%s_FULL", prefix);

    int shm_fd;       // file descriptor, used with shm_open()
    char *shm_base; // base address, used with mmap()

    /* open the shared memory segment as if it was a file */
    shm_fd = shm_open(name, O_RDWR, 0666);
    if (shm_fd == -1) {
        printf("cons: Shared memory failed: %s\n", strerror(errno));
        exit(1);
    }

    ftruncate(shm_fd, SIZE);
    /* map the shared memory segment to the address space of the process */
    shm_base = mmap(0, SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_base == MAP_FAILED) {
        printf("cons: Map failed: %s\n", strerror(errno));
        exit(1);   // Cleanup of shm_open was not done
    }

    /**
     * Open the two named semaphores
     */

    sem_t * empty_sem, *full_sem;

    // create and initialize the semaphore
    if ( (empty_sem = sem_open(nameEmpty, 0)) == SEM_FAILED)
        printf("Error opening %s: %s\n",nameEmpty, strerror(errno));
    if ( (full_sem = sem_open(nameFull, 0)) == SEM_FAILED)
        printf("Error opening %s: %s\n",nameFull, strerror(errno));

    int n = 1;
    int i;
    data = (int *)shm_base;
    h = 0;
    int first_time = 0;
    long long before_time;
    long long diference_time;

    
    //  printf("Size of INT %d\n", sizeof(int));
    while (!done) {
        // posicionarse en primera posición de la zona compartida.
        // espera por memoria compartida haya sido leída;
        
        if (sem_wait(empty_sem)!=0) 
                printf("Error waiting %s\n",strerror(errno));
        else {
            if (!first_time)
            {
                before_time = current_timestamp();
                first_time = 1;
            }
            // Condicion para terminar el programa segun el tiempo N
            diference_time = current_timestamp() - before_time;
            if (diference_time> (N*1000))
            {
                done = 1;
            break;
            }
            // Condicion para reutiizar la memoria compartida.
            if (h >= (SIZE/(2*sizeof(int))))
            {
                h = 0;
            }
            
            // se escribe procurando ir corriendo la direccion de memoria
            *(data+sizeof(int)*h) = -1;
            h++;
            *(data+sizeof(int)*h) = 1; 
            h++;

            // Avisa que la memoria compartida tiene datos nuevos;
            if (sem_post(full_sem)!=0)
                printf("Error posting %s\n",strerror(errno));
            n++;
        }
    }
    *(data) = 0; // Se escribe la señal para terminar
    if (sem_post(full_sem)!=0)
        printf("Error posting %s\n",strerror(errno));

    sem_close(empty_sem);
    sem_unlink(nameEmpty);
    sem_close(full_sem);  
    sem_unlink(nameFull);

    /* remove the mapped shared memory segment from the address space of the process */
    if (munmap(shm_base, SIZE) == -1) {
        printf("cons: Unmap failed: %s\n", strerror(errno));
        exit(1);
    }

    /* close the shared memory segment as if it was a file */
    if (close(shm_fd) == -1) {
        printf("cons: Close failed: %s\n", strerror(errno));
        exit(1);
    }


    return 0;
}
