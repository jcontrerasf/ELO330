#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <semaphore.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>

#include <sys/param.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>

#include <errno.h>

#define MAX_THREADS 20
#define MAX_BUFF_SIZE 256

sem_t x, y;
pthread_t tid;
pthread_t teller_threads[MAX_THREADS];
pthread_t visualizer_threads[MAX_THREADS];
int n_tellers = 0;
int n_visualizers = 0;

int A = 0;
int B = 0;

int A_last = 0;
int B_last = 0;
int max(int x, int y);
void recieveVote(void* param);
void refreshData(void* param);


int main(int argc, char *argv[])
{
  fd_set rset;
  // Initialize variables
  int serverSocket_t, newSocket_t;
  struct sockaddr_in serverAddr_t;
  struct sockaddr_storage serverStorage_t;

  int serverSocket_v, newSocket_v;
  struct sockaddr_in serverAddr_v;
  struct sockaddr_storage serverStorage_v;

  socklen_t addr_size_t, addr_size_v;
  sem_init(&x, 0, 1);
  sem_init(&y, 0, 1);

  serverSocket_t = socket(AF_INET, SOCK_STREAM, 0);
  serverAddr_t.sin_addr.s_addr = INADDR_ANY;
  serverAddr_t.sin_family = AF_INET;

  serverSocket_v = socket(AF_INET, SOCK_STREAM, 0);
  serverAddr_v.sin_addr.s_addr = INADDR_ANY;
  serverAddr_v.sin_family = AF_INET;
  if (argc == 2){
    serverAddr_t.sin_port = htons(atoi(argv[1]));
    serverAddr_v.sin_port = htons(atoi(argv[1])+1);
  }else{
    serverAddr_t.sin_port = htons(0);
    serverAddr_v.sin_port = htons(0);
  }

  // Bind the socket to the
  // address and port number.
  bind(serverSocket_t, (struct sockaddr*)&serverAddr_t, sizeof(serverAddr_t));
  bind(serverSocket_v, (struct sockaddr*)&serverAddr_v, sizeof(serverAddr_v));

  // Listen on the socket,
  // with 40 max connection
  // requests queued
  if (listen(serverSocket_t, 50) == 0)
      printf("Listening Tellers\n");
  else
      printf("Error\n");

  // Array for thread
  pthread_t tid_t[60];

  if (listen(serverSocket_v, 50) == 0)
      printf("Listening Visualizers\n");
  else
      printf("Error\n");

  // Array for thread
  pthread_t tid_v[60];

  FD_ZERO(&rset);

  int i = 0;
  int maxfdp1, tellerfd, visualizerfd, nready;

  maxfdp1 = max(serverSocket_t, serverSocket_v) + 1;
  for (;;) {

    FD_SET(serverSocket_t, &rset);
    FD_SET(serverSocket_v, &rset);

    // select the ready descriptor
    nready = select(maxfdp1, &rset, NULL, NULL, NULL);

    if (FD_ISSET(serverSocket_t, &rset)) {
      addr_size_t = sizeof(serverStorage_t);
      // Extract the first
      // connection in the queue
      newSocket_t = accept(serverSocket_t,
                          (struct sockaddr*)&serverStorage_t,
                          &addr_size_t);
      
      if (pthread_create(&teller_threads[i++], NULL, recieveVote, &newSocket_t) != 0)
        printf("Failed to create thread\n");
    
    }
    
    if (FD_ISSET(serverSocket_v, &rset)) {
      addr_size_v = sizeof(serverStorage_v);
      // Extract the first
      // connection in the queue
      newSocket_v = accept(serverSocket_v,
                          (struct sockaddr*)&serverStorage_v,
                          &addr_size_v);
      
      if (pthread_create(&visualizer_threads[i++], NULL, refreshData, &serverSocket_v) != 0)
        printf("Failed to create thread\n");

    }



    if (i >= 50) {
      // Update i
      i = 0;
      while (i < 50) {
        // Suspend execution of
        // the calling thread
        // until the target
        // thread terminates
        pthread_join(teller_threads[i++],
                      NULL);
        pthread_join(visualizer_threads[i++],
        NULL);
      }
      // Update i
      i = 0;
    }
  }

  return 0;

}

int max(int x, int y)
{
    if (x > y)
        return x;
    else
        return y;
}

void recieveVote(void* param)
{
    int psd =  *((int *) param);
    char buf[MAX_BUFF_SIZE];
    int rc;
   
    /*      get data from  clients and send it back */
    for (;;)
    {
        printf("\n...server is waiting...\n"); // el codigo no funciona sin este printf
        if ((rc = read(psd, buf, sizeof(buf))) < 0)
            perror("receiving stream  message");
        if (rc > 0)
        {
            // Se imprime lo que se recibe : SECCION EN LA QUE SUMAMOS VOTOS
            buf[rc] = '\0';
            if (strncmp(buf, "a", 1) == 0)
            {
                A = A+1;
                printf("Se recibe voto para A, total : %i", A);
            }

            if (strncmp(buf, "b", 1) == 0)
            {
                B = B+1;
                printf("Se recibe voto para B, total : %i", B);
            }
        }
        else
        {
           printf("Disconnected..\n");
           pthread_exit(NULL);
        }
    }
}


void refreshData(void* param)
{
    int psd =  *((int *) param);
    char buf[MAX_BUFF_SIZE];
    int rc;
    int initFlag = 0;
    /*      get data from  clients and send it back */
    for (;;)
    {
      if (initFlag == 0)
      {
        printf("\nPoniendo al dia al Visualizador...\n"); 
        sprintf(buf, "a,%d,b,%d\n", A, B);
        printf(buf);
        // send(psd, buf, sizeof(buf), 0);
        initFlag++;
      }else
      {
        if (A!= A_last || B!= B_last)
        {
          printf("\nActualizando info...\n"); 
          sprintf(buf, "a,%d,b,%d\n", A, B);
          printf(buf);
          // send(psd, buf, sizeof(buf), 0);
          A_last = A;
          B_last = B;
        }
      }
      // Encontrar condicional para llamar a estas funciones.
      // printf("Disconnected..\n");
      // pthread_exit(NULL);
        
    }
}













































