#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>

int done = 0;

void sig_handler(int signum){
    done = 1;
}


int main(int argc, char *argv[]){

    if(argc!=3) {
        printf("Uso: %s <prefix_name> <N>\n", argv[0]);
        exit(1);
    }

    char *fifo_name = argv[1];
    int N = atoi(argv[2]);

    //printf("nombre de fifo es %s \n", fifo_name);
    //printf("N es %i \n", N);

  


    int n, fd;

    /*
     * Open the FIFO for writing.  It was
     * created by the server.
     */
    if ((fd = open(fifo_name, O_WRONLY)) < 0) {
        perror("open");
        exit(1);
    }


    char buf[1];
    

    // ACTIVAR RELOJ 
    signal(SIGALRM,sig_handler);
    alarm(N);

    n=1;
    int i;
    while (!done) {
        for (i=0; i< n; i++){
            //escriba a fifo -1
 	    buf[0] = -1;
            write(fd, buf, 1);
            //escriba a fifo +1
	    buf[0] = 1;
            write(fd, buf, 1);
        }
	    n++;
    }
    
    buf[0] = 0;
    write(fd, buf, 1);


    close(fd);
    exit(0);
}
