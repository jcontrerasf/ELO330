/*

NAME:       TCPCleint 
SYNOPSIS:    TCPCleint hostid portnumber 
DESCRIPTION:    The program creates a stream socket in the inet domain, 
                Connects to TCPServer, 
                Gets messsages typed by a user and 
                Sends them to TCPServer running on hostid
                Then it waits for a reply from  the TCPServer
                and shows it back to the user, with a message
                indicating if there is an error during the round trip. 
*/
#include <sys/socket.h>  /* socket, connect, recv, send */
#include <arpa/inet.h>   /* htons  */
#include <netinet/in.h>  /* inet_addr */
#include <arpa/inet.h>   /* inet_addr */
#include <string.h>   /* memcpy  */
#include <stdio.h>   /*printf */
#include <unistd.h>   /* close */
#include <stdlib.h>  /* exit  */
#include <netdb.h>  /* gethostbyaddr  */

#define MAXHOSTNAME 80
#define BUFSIZE 1024
char buf[BUFSIZE];
char rbuf[BUFSIZE];
int cleanup(char * );

int main( int argc, char * argv[] ) {
  int   sd;
  struct   sockaddr_in server;
  struct  hostent *hp, *gethostbyname();
  struct  servent *sp;
  struct sockaddr_in from;
  struct sockaddr_in addr;
  int fromlen;
  int length;
  int rc, cc;
  char ThisHost[80];

  sp = getservbyname("echo", "tcp");
  /* get TCPClient Host information, NAME and INET ADDRESS */

  gethostname(ThisHost, MAXHOSTNAME);

  printf("----TCP/Cleint running at host NAME: %s\n", ThisHost);
  if  ( (hp = gethostbyname(ThisHost)) == NULL ) {
    fprintf(stderr, "Can't find host %s\n", ThisHost);
    exit(-1);
  }
  memcpy(&(server.sin_addr), hp->h_addr, hp->h_length);
  printf("    (TCP/Cleint INET ADDRESS is: %s )\n", inet_ntoa(server.sin_addr));

  /* get TCP/Server Host information, NAME and INET ADDRESS */

  if  ( (hp = gethostbyname(argv[1])) == NULL ) {
    addr.sin_addr.s_addr = inet_addr(argv[1]);
    if ((hp = gethostbyaddr((void *)&addr.sin_addr.s_addr,
        sizeof(addr.sin_addr.s_addr),AF_INET)) == NULL) {
      fprintf(stderr, "Can't find host %s\n", argv[1]);
      exit(-1);
    }
  }
  printf("----TCP/Server running at host NAME: %s\n", hp->h_name);
  memcpy(&(server.sin_addr), hp->h_addr, hp->h_length);
  printf("    (TCP/Server INET ADDRESS is: %s )\n", inet_ntoa(server.sin_addr));

  /* Construct name of socket to send to. */
  server.sin_family = hp->h_addrtype;
  /* server.sin_family = AF_INET; */

  server.sin_port = htons(atoi(argv[2]));
  /*OR    server.sin_port = sp->s_port; */

  /*   Create socket on which to send  and receive */

  sd = socket (hp->h_addrtype,SOCK_STREAM,0);
  /*OR  sd = socket (PF_INET,SOCK_STREAM,0); */

  if (sd<0) {
    perror("opening stream socket");
    exit(-1);
  }
  /* Connect to TCP/SERVER */
  if ( connect(sd, (struct sockaddr *)&server, sizeof(server)) < 0 ) {
    close(sd);
    perror("connecting stream socket");
    exit(0);
  }
  fromlen = sizeof(from);
  if (getpeername(sd, (struct sockaddr *)&from,&fromlen)<0){
    perror("could't get peername\n");
    exit(1);
  }
  printf("Connected to TCP/Server:");
  printf("%s:%d\n", inet_ntoa(from.sin_addr),
      ntohs(from.sin_port));
  if ((hp = gethostbyaddr((void *)&from.sin_addr.s_addr,
      sizeof(from.sin_addr.s_addr),AF_INET)) == NULL)
    fprintf(stderr, "Can't find host %s\n", inet_ntoa(from.sin_addr));
  else
    printf("(Name is : %s)\n", hp->h_name);


  /*     get data from USER, send it SERVER,
      receive it from SERVER, display it back to USER  */

  for(;;) {
    printf("\nType anything followed by RETURN, or type CTRL-D to exit\n");
    cleanup(buf);
    cleanup(rbuf);
    rc=read(0,buf, sizeof(buf));
    if (rc == 0) break;
    if (send(sd, buf, rc, 0) <0 )
      perror("sending stream message");
    if ((cc=recv(sd, rbuf, sizeof(rbuf), 0)) < 0)
      perror("receiving echo");
    if (cc == rc)
      if (strcmp(buf, rbuf) == 0){
        printf("Echo is good\n");
        printf("  Received: %s", rbuf);
      }
      else
        printf("Echo bad -- strings unequal: %s\n", rbuf);
    else
      printf("Echo bad -- lengths unequal: %s\n", rbuf);
  }
  printf ("EOF... exit\n");
  close(sd);
  exit (0);
}
int cleanup(char * buf)  {
  int i;
  for(i=0; i<BUFSIZE; i++) buf[i]='\0';
}
