// se cambio maxfdp1 = max(serverSocket_t, serverSocket_v) + 1
// maxfdp1 = serverSocket_t + 1



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


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

#define ARCHIVO "/WWW/resultados.html"

sem_t x, y;
pthread_t tid;
pthread_t teller_threads[MAX_THREADS];
int n_tellers = 0;


int max(int x, int y);
void *gameClient(void *param);
void *refreshData(void *param);

void generarMazo(int *mazo);
int sumarCartas(int *mano, int Top, int CartaInicial);
void shuffle(int *array, size_t n);
void Push(int *Top, int Size, int *inp_array, int x);
int Pop(int *Top, int *inp_array);
void resultadosHTML(char* jugador, int puntajeJugador, int puntajeCPU);


pthread_mutex_t lock;

int main(int argc, char *argv[])
{
  fd_set rset;
  // Initialize variables
  char hostname[64];
  struct hostent *hp;
  int serverSocket_t, newSocket_t;
  struct sockaddr_in serverAddr_t;
  struct sockaddr_storage serverStorage_t;

  socklen_t addr_size_t;
  sem_init(&x, 0, 1);
  sem_init(&y, 0, 1);
  gethostname(hostname, sizeof(hostname));
  // strcpy(hostname, "localhost");
  hp = gethostbyname(hostname);

  serverSocket_t = socket(AF_INET, SOCK_STREAM, 0);

  serverAddr_t.sin_addr.s_addr = INADDR_ANY;
  serverAddr_t.sin_family = AF_INET;



  if (argc == 2)
  {
    serverAddr_t.sin_port = htons(atoi(argv[1]));
  }
  else
  {
    serverAddr_t.sin_port = htons(0);
  }

  // Bind the socket to the
  // address and port number.
  bind(serverSocket_t, (struct sockaddr *)&serverAddr_t, sizeof(serverAddr_t));

  // Listen on the socket,
  // with 40 max connection
  // requests queued
  if (listen(serverSocket_t, 50) == 0)
    printf("Listening Tellers\n");
  else
    printf("Error\n");

  // Array for thread
  pthread_t tid_t[60];


  FD_ZERO(&rset);

  int i = 0;
  int maxfdp1, tellerfd, visualizerfd, nready;

  //maxfdp1 = max(serverSocket_t, serverSocket_v) + 1;
  maxfdp1 = serverSocket_t + 1;

  if (pthread_mutex_init(&lock, NULL) != 0)
  {
    printf("\n mutex init has failed\n");
    return 1;
  }

  for (;;)
  {

    FD_SET(serverSocket_t, &rset);

    // select the ready descriptor
    nready = select(maxfdp1, &rset, NULL, NULL, NULL);

    if (FD_ISSET(serverSocket_t, &rset))
    {
      addr_size_t = sizeof(serverStorage_t);
      // Extract the first
      // connection in the queue
      newSocket_t = accept(serverSocket_t,
                           (struct sockaddr *)&serverStorage_t,
                           &addr_size_t);

      if (pthread_create(&teller_threads[i++], NULL, gameClient, &newSocket_t) != 0)
        printf("Failed to create thread\n");
    }

   
    if (i >= 50)
    {
      // Update i
      i = 0;
      while (i < 50)
      {
        // Suspend execution of
        // the calling thread
        // until the target
        // thread terminates
        pthread_join(teller_threads[i++],
                     NULL);
      }
      // Update i
      i = 0;
    }
  }
  pthread_mutex_destroy(&lock);
  return 0;
}

int max(int x, int y)
{
  if (x > y)
    return x;
  else
    return y;
}






enum Estado
{
  Standby,
  MitadJuego
};

void *gameClient(void *param)
{
  int mazo[52];
  int Top;

  int manoCliente[12];
  int TopManoCliente;

  int manoServidor[12];
  int TopManoServidor;

  enum Estado estado;
  estado = Standby;

  // variables para obtener el mensaje
  int psd = *((int *)param);
  char buf[MAX_BUFF_SIZE];
  char buf_send[MAX_BUFF_SIZE];
  int rc;
  int j = 0;
  /*      get data from  clients and send it back */
  for (;;)
  {
    printf("\n...server is waiting...\n"); // el codigo no funciona sin este printf
    if ((rc = read(psd, buf, sizeof(buf))) < 0)
      perror("receiving stream  message");
    if (rc > 0) // MENSAJE RECIBIDO DESDE EL CLIENTE
    {
      buf[rc] = '\0';

      // SI EL JUGADOR PIDE INICIAR EL JUEGO
      if ((strncmp(buf, "I", 1) == 0) && estado == Standby)
      {
        // generar mazo de 52 cartas
        generarMazo(mazo);
        Top = 51; 
        TopManoCliente = -1;
        TopManoServidor = -1;
        // repartir 2 al jugador
        Push(&TopManoCliente, 12, manoCliente, Pop(&Top, mazo));
        printf("Jugador recibe la carta : %d \n", manoCliente[TopManoCliente]);
        sprintf(buf_send, "%d\n", manoCliente[TopManoCliente]);
        send(psd, buf_send, sizeof(buf_send), 0);
        for(j = 0; j < SIZE_MAX; j++)
          buf_send[j] = NULL;

        Push(&TopManoCliente, 12, manoCliente, Pop(&Top, mazo));
        printf("Jugador recibe la carta : %d \n", manoCliente[TopManoCliente]);
        sprintf(buf_send, "%d\n", manoCliente[TopManoCliente]);
        send(psd, buf_send, sizeof(buf_send), 0);
        for(j = 0; j < SIZE_MAX; j++)
          buf_send[j] = NULL;
        // repartir 2 al servidor

        Push(&TopManoServidor, 12, manoServidor, Pop(&Top, mazo));
        printf("Servidor recibe la carta : %d \n", manoServidor[TopManoServidor]);

        Push(&TopManoServidor, 12, manoServidor, Pop(&Top, mazo));
        printf("Servidor recibe la carta : %d \n", manoServidor[TopManoServidor]);
        // pasar al siguiente estado
        estado = MitadJuego;
      }

      // SI EL JUGADOR PIDE SACAR CARTA
      if ((strncmp(buf, "S", 1) == 0) && estado == MitadJuego)
      {
        // revisar que las cartas visibles del cliente no superen 21 (si no lo superan mandarle una carta, si lo superan mandarle una "N")
        // tenemos que revisar las cartas desde la que esta en la posicion 1 hasta la que esta en la posicion TopManoCliente
        if (sumarCartas(manoCliente, TopManoCliente, 1) >= 21)
        {
          // Enviar una N
          printf("El jugador ya no puede pedir mas cartas \n");
          sprintf(buf_send, "N\n");
          send(psd, buf_send, sizeof(buf_send), 0);
          for(j = 0; j < SIZE_MAX; j++)
            buf_send[j] = NULL;
        }
        else
        {
          Push(&TopManoCliente, 12, manoCliente, Pop(&Top, mazo));
          printf("Jugador pidio y recibe la carta : %d \n", manoCliente[TopManoCliente]);

          sprintf(buf_send, "%d\n", manoCliente[TopManoCliente]);
          send(psd, buf_send, sizeof(buf_send), 0);
          for(j = 0; j < SIZE_MAX; j++)
            buf_send[j] = NULL;
        }
      }

      // SI EL JUGADOR PIDE TERMINAR EL JUEGO
      if ((strncmp(buf, "R", 1) == 0) && estado == MitadJuego)
      {
        // sacar cartas para el servidor hasta tener 17 o mas
        while (sumarCartas(manoServidor, TopManoServidor, 0) <= 16)
        {
          Push(&TopManoServidor, 12, manoServidor, Pop(&Top, mazo));
          printf("Servidor recibe la carta : %d \n", manoServidor[TopManoServidor]);
        }



        // Retornar "L" si el cliente tiene mas que 21, retornar "L" si el servidor tiene mas o igual que el cliente y menos o igual a 21, retornar "W" en cualquier otro caso
        if (sumarCartas(manoCliente, TopManoCliente, 0) > 21)
        {
          // Enviar "L"
          sprintf(buf_send, "L\n");
          send(psd, buf_send, sizeof(buf_send), 0);
          for(j = 0; j < SIZE_MAX; j++)
            buf_send[j] = NULL;
          printf("El jugador supero 21  \n");
        }
        else if (sumarCartas(manoServidor, TopManoServidor, 0) >= sumarCartas(manoCliente, TopManoCliente, 0) && sumarCartas(manoServidor, TopManoServidor, 0) <=21) //REFERENCIATE ACA JULIO, PUNTAJE DE SERVER Y CLIENTE
        {
          // Enviar "L"
          sprintf(buf_send, "L\n");
          send(psd, buf_send, sizeof(buf_send), 0);
          for(j = 0; j < SIZE_MAX; j++)
            buf_send[j] = NULL;
          printf("El jugador no supero al dealer\n");
        }
        else
        {
          // Enviar "W"
          sprintf(buf_send, "W\n");
          send(psd, buf_send, sizeof(buf_send), 0);
          for(j = 0; j < SIZE_MAX; j++)
            buf_send[j] = NULL;
          printf("El jugador gano\n");
        }

        // Retornar el puntaje que recibio el servidor
        sprintf(buf_send, "%d\n", sumarCartas(manoServidor, TopManoServidor, 0));
        send(psd, buf_send, sizeof(buf_send), 0);
        for(j = 0; j < SIZE_MAX; j++)
            buf_send[j] = NULL;
        printf("Se envia el puntaje que obtuvo el dealer \n");


        resultadosHTML("Nombre Jugador", sumarCartas(manoCliente, TopManoCliente, 0), sumarCartas(manoServidor, TopManoServidor, 0));


        // Cambiar estado a Standby
        estado = Standby;
      }
    }
    else
    {
      printf("Disconnected..\n");
      pthread_exit(NULL);
    }
  }
}

void generarMazo(int *mazo)
{
  int i;
  int j;
  // Primero llenemos el mazo con valores
  for (i = 1; i <= 10; i++)
  {
    if (i <= 9)
    {
      for (j = 0; j < 4; j++)
      {
        mazo[4 * (i - 1) + j] = i;
      }
    }
    else
    {
      for (j = 0; j < 16; j++)
      {
        mazo[4 * (i - 1) + j] = i;
      }
    }
  }
  // Ahora lo randomizamos
  shuffle(mazo, 52);
}


int sumarCartas(int *mano, int Top, int CartaInicial)
{
  bool Az = false;
  int suma = 0;
  int i;
  // Primero debemos identificar si es que existe un AZ en la mano
  for (i = CartaInicial; i <= Top; i++)
  {
    if (mano[i] == 1)
      Az = true;
  }
  // Ahora contamos las cartas
  if (Az == false)
  {
    for (i = CartaInicial; i <= Top; i++)
    {
      suma += mano[i];
    }
  }
  else
  {
    for (i = CartaInicial; i <= Top; i++)
    {
      suma += mano[i];
    }
    if (suma + 10 <= 21)
      suma += 10;
  }
  return suma;
}


/* Arrange the N elements of ARRAY in random order.
   Only effective if N is much smaller than RAND_MAX;
   if this may not be the case, use a better random
   number generator. */
void shuffle(int *array, size_t n)
{
  if (n > 1)
  {
    size_t i;
    for (i = 0; i < n - 1; i++)
    {
      size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
      int t = array[j];
      array[j] = array[i];
      array[i] = t;
    }
  }
}

void Push(int *Top, int Size, int *inp_array, int x)
{
  if (*Top == Size - 1)
  {
    printf("Overflow!! \n");
  }
  else
  {
    *Top = *Top + 1;
    inp_array[*Top] = x;
  }
}

int Pop(int *Top, int *inp_array)
{
  int popped;
  if (*Top == -1)
  {
    printf("Underflow!! \n");
  }
  else
  {
    popped = inp_array[*Top];
    *Top = *Top - 1;
    return popped;
  }
}




void resultadosHTML(char* jugador, int puntajeJugador, int puntajeCPU){

   char fp_buff[256]; //file path buffer
   FILE *file;
   int archivo_existe = 0;
   char* ganador;
   time_t t = time(NULL);
   struct tm tm = *localtime(&t);

   strcat(strcpy(fp_buff, getenv("HOME")), ARCHIVO);

   if ((file = fopen(fp_buff, "r")))
   {
      archivo_existe = 1;
      fclose(file);
   }

   file = fopen(fp_buff, "a");

   if(!archivo_existe){
      fprintf(file,
                  "<title>Resultados Black Jack</title>\r\n"
                  "<header>\r\n"
                  "   <h1>Resultados Black Jack</h1>\r\n"
                  "   <p>Proyecto final ELO330</p>\r\n"
                  "</header>\r\n");
   }

   if (puntajeJugador > puntajeCPU)
   {
      ganador = jugador;
   }else{
      ganador = "CPU";
   }

   
   fprintf(file,
               "<div>\r\n"
               "  <h3>Resultados del juego del %02d/%02d/%d a las %02d:%02d</h3>\r\n"
               "  <p>Ganador: %s</p>\r\n"
               "  <table class='resultado'>\r\n"
               "    <thead>\r\n"
               "      <tr>\r\n"
               "        <th>Jugador</th>\r\n"
               "        <th>Puntaje</th>\r\n"
               "      </tr>\r\n"
               "    </thead>\r\n"
               "    <tbody>\r\n"
               "      <tr>\r\n"
               "        <td>%s</td>\r\n"
               "        <td>%d</td>\r\n"
               "      </tr>\r\n"
               "      <tr>\r\n"
               "        <td>CPU</td>\r\n"
               "        <td>%d</td>\r\n"
               "      </tr>\r\n"
               "    <tbody>\r\n"
               "  </table>\r\n"
               "</div>\r\n"
      , tm.tm_mday, tm.tm_mon+1, tm.tm_year+1900, tm.tm_hour, tm.tm_min, ganador, jugador, puntajeJugador, puntajeCPU);
   
   fclose(file);
}






/*
void *refreshData(void *param)
{
  int psd = *((int *)param);
  char buf[MAX_BUFF_SIZE];
  int rc;
  int initFlag = 0;
  /*      get data from  clients and send it back */


  /*
  for (;;)
  {
    if (initFlag == 0)
    {
      printf("\nPoniendo al dia al Visualizador...\n");
      sprintf(buf, "a,%d,b,%d\n", A, B);
      printf(buf);
      send(psd, buf, sizeof(buf), 0);
      initFlag++;
    }
    else
    {
      if (A != A_last || B != B_last)
      {
        printf("\nActualizando info...\n");
        sprintf(buf, "a,%d,b,%d\n", A, B);
        printf(buf);
        send(psd, buf, sizeof(buf), 0);
        A_last = A;
        B_last = B;
      }
    }
    // Encontrar condicional para llamar a estas funciones.
    // printf("Disconnected..\n");
    // pthread_exit(NULL);
  }
}
*/
