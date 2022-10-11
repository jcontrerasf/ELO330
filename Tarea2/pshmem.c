/*

cshmem: consume datos desde memoria compartida

Sintaxis: pshmem <prefix_name> <N>

Descripción: 

Compilar con: gcc -o pshmem pshmem.c -lrt

*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <errno.h>

int done = 0;

void sig_handler(int signum){
    done = 1;
}


int main(int argc, char *argv[])
{
    if(argc!=3) {
        printf("Uso: cshmem <prefix_name> <N>\n");
        exit(1);
    }

    const int SIZE = 4096;      // file size
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


    /* open the shared memory segment as if it was a file */
    shm_fd = shm_open(name, O_RDONLY, 0666);
    if (shm_fd == -1) {
        printf("cons: Shared memory failed: %s\n", strerror(errno));
        exit(1);
    }

    /* map the shared memory segment to the address space of the process */
    shm_base = mmap(0, SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_base == MAP_FAILED) {
        printf("cons: Map failed: %s\n", strerror(errno));
        exit(1);   // Cleanup of shm_open was not done
    }

    /**
     * Open the two named semaphores
     */

    // create and initialize the semaphore
    if ( (empty_sem = sem_open(nameEmpty, 0)) == SEM_FAILED)
        printf("Error opening %s: %s\n",nameEmpty, strerror(errno));
    if ( (full_sem = sem_open(nameFull, 0)) == SEM_FAILED)
        printf("Error opening %s: %s\n",nameFull, strerror(errno));



    // ACTIVAR RELOJ 
    signal(SIGALRM, sig_handler);
    alarm(N);
    

    char buf[1];
    n = 1;
    int i;
    while (!done) {
        // posicionarse en primera posición de la zona compartida.
        // espera por memoria compartida haya sido leída;
        
        if (sem_wait(empty_sem)!=0) 
                printf("Error waiting %s\n",strerror(errno));
        else {
            for (i=0; i< n; i++){
                // escriba a shmem -1;
                buf[0] = -1;
                write(fd, buf, 1);

                //escriba a shmem +1
                buf[0] = 1;
                write(fd, buf, 1);
                
                nc = write(STDIN_FILENO, shm_base, SIZE-1);
                shm_base[nc] ='\0';  /* to make it NULL termited string */
                strncpy(shm_base, "-1", SIZE);
            }
            // Avisa que la memoria compartida tiene datos nuevos;
            if (sem_post(full_sem)!=0)
                        printf("Error posting %s\n",strerror(errno));
            n++;
        }
    }

    buf[0] = 0;
    write(fd, buf, 1);


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