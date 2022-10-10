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


    char buf[1];
    

    // ACTIVAR RELOJ 
    signal(SIGALRM, sig_handler);
    alarm(N);

    n=1;
    int i;
    while (!done) {
        for (i=0; i< n; i++){
            //escriba a shmem -1
        buf[0] = -1;
            write(fd, buf, 1);
            //escriba a shmem +1
        buf[0] = 1;
            write(fd, buf, 1);
        }
        n++;
    }
    
    buf[0] = 0;
    write(fd, buf, 1);



    return 0;


}