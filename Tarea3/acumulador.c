/*
NAME:       TCPServer [Port number]
SYNOPSIS:    TCPServer 
DESCRIPTION:  The program creates a TCP socket in the inet, 
              listens for connections from TCPClients, accepts
              clients into private sockets, and forks
              an echo process to ``serve'' the cleint.
*/

#include <stdio.h>
#include <errno.h>
#include <sys/time.h>

#include <sys/param.h>
#include <sys/socket.h>
#include <sys/file.h>

#include <netinet/in_systm.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <strings.h>
#include <stdlib.h>

#include <unistd.h>  /*gethostname*/

#define MAXHOSTNAME 80
void reusePort(int sock);   
void EchoServe(int psd, struct sockaddr_in from);

int main(int argc,  char *argv[]) {
  int   sd, psd;
  struct   sockaddr_in server;
  struct  hostent *hp, *gethostbyname();
  struct  servent *sp;
  struct sockaddr_in from;
  int fromlen;
  int length;
  char ThisHost[MAXHOSTNAME];

  sp = getservbyname("echo", "tcp");
  /* get EchoServer Host information, NAME and INET ADDRESS */

  gethostname(ThisHost, MAXHOSTNAME);

  printf("----TCP/Server running at host NAME: %s\n", ThisHost);
  if  ( (hp = gethostbyname(ThisHost)) == NULL ) {
    fprintf(stderr, "Can't find host %s\n", ThisHost);
    exit(-1);
  }
  bcopy ( hp->h_addr, &(server.sin_addr), hp->h_length);
  printf("    (TCP/Server INET ADDRESS is: %s )\n", inet_ntoa(server.sin_addr));


  /* Construct name of socket to send to. */
  server.sin_family = hp->h_addrtype;
  /*OR server.sin_family = AF_INET; */


  server.sin_addr.s_addr = htonl(INADDR_ANY);

  /*OR    server.sin_port = sp->s_port; */
  if (argc == 2)
      server.sin_port = htons(atoi(argv[1])); 
  else
      server.sin_port = htons(0);  

  /*   Create socket on which to send  and receive */

  /* sd = socket (PF_INET,SOCK_DGRAM,0); */
  sd = socket (hp->h_addrtype,SOCK_STREAM,0);

  if (sd<0) {
    perror("opening stream socket");
    exit(-1);
  }

  reusePort(sd);

  if ( bind( sd, (struct sockaddr *) &server, sizeof(server) ) ) {
    close(sd);
    perror("binding name to stream socket");
    exit(-1);
  }

  length = sizeof(server);
  if ( getsockname (sd, (struct sockaddr *) &server,&length) ) {
    perror("getting socket name");
    exit(0);
  }

  printf("Server Port is: %d\n", ntohs(server.sin_port));
  listen(sd,4);
  fromlen = sizeof(from);
  for(;;){
    psd  = accept(sd, (struct sockaddr *) &from, &fromlen);
    if ( fork() == 0) {
      close (sd);
      EchoServe(psd, from);
    }
  }
}

void EchoServe(int psd, struct sockaddr_in from) {
  char buf[512];
  int rc;
  struct  hostent *hp, *gethostbyname();

  printf("Serving %s:%d\n", inet_ntoa(from.sin_addr),
      ntohs(from.sin_port));
  if ((hp = gethostbyaddr((char*)&from.sin_addr.s_addr,
      sizeof(from.sin_addr.s_addr),AF_INET)) == NULL)
    fprintf(stderr, "Can't find host %s\n", inet_ntoa(from.sin_addr));
  else
    printf("(Name is : %s)\n", hp->h_name);


  /*      get data from  clients and send it back */
  for(;;){
    printf("\n...server is waiting...\n");
    if( (rc=read(psd, buf, sizeof(buf))) < 0)
      perror("receiving stream  message");
    if (rc > 0){
      buf[rc]='\0';
      printf("Received: \"%s\"\n", buf);
      printf("From TCP/Client: %s:%d\n", inet_ntoa(from.sin_addr),
          ntohs(from.sin_port));
      printf("(Name is : %s)\n", hp->h_name);
      if (send(psd, buf, rc, 0) <0 )
        perror("sending stream message");
    }
    else {
      printf("TCP/Client: %s:%d\n", inet_ntoa(from.sin_addr),
          ntohs(from.sin_port));
      printf("(Name is : %s)\n", hp->h_name);
      printf("Disconnected..\n");
      close (psd);
      exit(0);
    }
  }
}
void reusePort(int s)
{
  int one=1;

  if ( setsockopt(s,SOL_SOCKET,SO_REUSEADDR,(char *) &one,sizeof(one)) == -1 )
  {
    printf("error in setsockopt,SO_REUSEPORT \n");
    exit(-1);
  }
}      
