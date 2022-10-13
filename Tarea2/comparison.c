#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#define BARCO "barco.txt"

int main(void) {
    pid_t pid;
    int pfd[2];
    int i, status;
    FILE * sd;
    
    /*
     * Create a pipe.
     */
    if (pipe(pfd) < 0) {
        perror("pipe");
        exit(1);
    }

    /*
     * Create a child process.
     */
    if ((pid = fork()) < 0) {
        perror("fork");
        exit(1);
    }

    /*
     * The child process executes "gnuplot".
     */
    if (pid == 0) {
        /*
         * Attach standard input of this child process to read from the pipe.
         */
        dup2(pfd[0], 0);
        close(pfd[1]);  /* close the write end off the pipe */

        execlp("gnuplot", "gnuplot", NULL);
        perror("exec");
        _exit(127);
    }

    /*
     * We won't be reading from the pipe.
     */
    close(pfd[0]);
    sd = fdopen(pfd[1], "w");
    fprintf(sd, "plot [0:30] [0:25] \"%s\" using ($1+%i):($2+%i) with lines lt 1,", BARCO, 15, 17);
    fprintf(sd, " \"%s\" using ($2+%i):($1+%i) with lines lt 2\n", BARCO, 2, 3);
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
    
    /*
     * Close the pipe and wait for the child
     * to exit.
     */
    fclose(sd);
    waitpid(pid, &status, 0);

    /*
     * Exit with a status of 0, indicating that
     * everything went fine.
     */
    exit(0);
}
