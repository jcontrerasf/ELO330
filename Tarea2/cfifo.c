#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>


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

int main(int argc, char *argv[]){

    if(argc!=3){
        perror("Numero incorrecto de argumentos de linea de comando");
        exit(1);
    }


    char *fifo_name = argv[1];
    int N = atoi(argv[2]);

    printf("nombre de fifo es %s \n", fifo_name);
    printf("N es %i \n", N);

    tasa_medida = (int *)malloc(10*N*sizeof(char));



    
    struct itimerval timer;
    

    signal(SIGALRM, sig_handler);

    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 100000;

    timer.it_interval.tv_sec = 0; 
    timer.it_interval.tv_usec = 100000;

	 

    int n, fd;
    char buf[1];
    /*
     * Remove any previous FIFO.
     */
    unlink(fifo_name);

    /* Create the FIFO.*/
    if (mkfifo(fifo_name, 0666) < 0) {
        perror("mkfifo");
        exit(1);
    }

     /* Open the FIFO for reading.*/
    if ((fd = open(fifo_name, O_RDONLY)) < 0) {
        perror("open");
        exit(1);
    }


    int sum = 0;
    int i;
    int primera_vez = 0;
    /* Read from the FIFO until end-of-file and
        print on the standard output. */


    n = read(fd, buf, sizeof(buf));

    setitimer(ITIMER_REAL, &timer, NULL);

    do{
        sum += (int)(buf[0]);
        contador_bytes++;
        if((int)(buf[0]) == 0)
            break;
       

    } while (n = read(fd, buf, sizeof(buf)));


    // Imprimir suma total
    printf("La suma total es : %i \n", sum);
    //Imprimir tasas
    for(i=0; i<10*N; i++)
    printf("La tasa de bytes en el tramo %i es : %i [bytes/segundo] \n", i+1, tasa_medida[i]);

    close(fd);
    exit(0);


}

