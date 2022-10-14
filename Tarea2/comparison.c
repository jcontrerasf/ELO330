#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>

//#define FIFO_OUT_FILE "fifo_out.txt"
//#define TEST_FILE "test.txt"


int main(int argc, char *argv[]) {
    pid_t pid_plot, pid_cfifo, pid_pfifo, pid_cshmem, pid_pshmem;
    int pipe_plot[2];
    int i, status;
    FILE * sd;
    int fork_done = 0;

    if(argc!=3) {
        printf("Uso: %s <prefix_name> <N>\n", argv[0]);
        exit(1);
    }

    char * const cfifo_args[] = { "cfifo", argv[1], argv[2] , NULL};
    char * const pfifo_args[] = { "pfifo", argv[1], argv[2] , NULL};

    char * const cshmem_args[] = { "cshmem", argv[1], argv[2] , NULL};
    char * const pshmem_args[] = { "pshmem", argv[1], argv[2] , NULL};

    char fifo_filename[100]; // file name
    sprintf(fifo_filename, "%s_fifo_out.txt", argv[1]);

    char shmem_filename[100]; // file name
    sprintf(shmem_filename, "%s_shmem_out.txt", argv[1]);


    int fifo_out_fd = open(fifo_filename, O_RDWR | O_CREAT, 0666);

    int shmem_out_fd = open(shmem_filename, O_RDWR | O_CREAT, 0666);
    
    /*
     * Create a pipe.
     */
    if (pipe(pipe_plot) < 0) {
        perror("pipe");
        exit(1);
    }


    /*
     * Create a child process.
     */
    if ((pid_plot = fork()) < 0) {
        perror("fork");
        exit(1);
    }

    /*
     * The child process executes "gnuplot".
     */
    if (pid_plot == 0) {
        /*
         * Attach standard input of this child process to read from the pipe.
         */
        dup2(pipe_plot[0], 0);
        close(pipe_plot[1]);  /* close the write end off the pipe */

        execlp("gnuplot", "gnuplot", NULL);
        perror("exec");
        _exit(127);
    }

    if ((pid_cfifo = fork()) < 0) {
        perror("fork");
        exit(1);
    }else{
        fork_done = 1;
    }

    if(pid_cfifo == 0){
        printf("inicio cfifo\n");
        dup2(fifo_out_fd, 1); // redirect stdout to file

        execvp("./cfifo", cfifo_args);
        perror("exec");
    }
    sleep(1);

    if(fork_done && pid_cfifo!=0){
        if ((pid_pfifo = fork()) < 0) {
            perror("fork");
            exit(1);
        }
    }

    
    if(pid_pfifo == 0){
        printf("inicio pfifo\n");
        execvp("./pfifo", pfifo_args);
        perror("exec");
    }


    
    waitpid(pid_cfifo, &status, 0);
    waitpid(pid_pfifo, &status, 0);

    fork_done = 0;


    if ((pid_cshmem = fork()) < 0) {
        perror("fork");
        exit(1);
    }else{
        fork_done = 1;
    }

    if(pid_cshmem == 0){
        printf("inicio cshmem\n");
        dup2(shmem_out_fd, 1); // redirect stdout to file

        execvp("./cshmem", cshmem_args);
        perror("exec");
    }
    sleep(1);

    if(fork_done && pid_cshmem!=0){
        if ((pid_pshmem = fork()) < 0) {
            perror("fork");
            exit(1);
        }
    }

    
    if(pid_pshmem == 0){
        printf("inicio pshmem\n");
        execvp("./pshmem", pshmem_args);
        perror("exec");
    }


    
    waitpid(pid_cshmem, &status, 0);
    waitpid(pid_pshmem, &status, 0);

    /*
     * We won't be reading from the pipe.
     */
    close(pipe_plot[0]);
    sd = fdopen(pipe_plot[1], "w");

    FILE *fifo_file = fopen(fifo_filename, "r");
    char buf[10];
    fgets(buf, 10, fifo_file);
    strtok(buf, "\n");

    if(buf[0]=='0'){
        printf("El resultado de la suma fifo es 0\n");
    }else{
        printf("El resultado es distinto de cero %s\n", buf);
    }

    buf[0] = 1;
    FILE *shmem_file = fopen(shmem_filename, "r");
    fgets(buf, 10, shmem_file);
    strtok(buf, "\n");

    if(buf[0]=='0'){
        printf("El resultado de la suma shmem es 0\n");
    }else{
        printf("El resultado es distinto de cero %s\n", buf);
    }
    
    fprintf(sd,"set ylabel \"Tasa de transmision [B/s]\"\nset xlabel\"Tiempo [ds]\"\nset title \"Comparacion de velocidad de transmision entre fifo y memoria compartida\"\n");
    fprintf(sd, "plot \"%s\" every ::1 with lines lt 3 title \"fifo\", \"%s\" every ::1 with lines lt 1 title \"shmem\"\n pause 5\n", fifo_filename, shmem_filename);
    fflush(sd);
    //fprintf(sd, "pause 100 \n"); fflush(sd);
 #if 0   
    fprintf(sd, " \"%s\" using ($2+%i):($1+%i) with lines lt 2\n", FIFO_OUT_FILE, 2, 3);
/*    fprintf(sd, "pause -1 \n"); */ fflush(sd);
/*    getchar();*/
    sleep(5);

    /*fprintf(sd, "\n"); fflush(sd);*/
    fprintf(sd, "plot [0:30] [0:25] \"%s\" using ($1+%i):($2+%i) with lines  lt 1,", BARCO, 14, 17);
    fprintf(sd, " \"%s\" using ($1+%i):($2+%i) with lines lt 2\n", BARCO, 3, 3);
    fprintf(sd, "pause -1 \n"); fflush(sd);
    sleep(5);
    
      /*  getchar();*/

    fprintf(sd, "\n exit"); fflush(sd);
    #endif
    /*
     * Close the pipe and wait for the child
     * to exit.
     */
    fclose(sd);
    waitpid(pid_plot, &status, 0);
    
    
    close(fifo_out_fd);
    fclose(fifo_file);
    unlink(fifo_filename);

    close(shmem_out_fd);
    fclose(shmem_file);
    unlink(shmem_filename);

    /*
     * Exit with a status of 0, indicating that
     * everything went fine.
     */
    exit(0);
}
