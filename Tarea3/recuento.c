#include <sys/socket.h> /* socket, connect, recv, send */
#include <arpa/inet.h>  /* htons  */
#include <netinet/in.h> /* inet_addr */
#include <arpa/inet.h>  /* inet_addr */
#include <string.h>     /* memcpy  */
#include <stdio.h>      /*printf */
#include <unistd.h>     /* close */
#include <stdlib.h>     /* exit  */
#include <netdb.h>      /* gethostbyaddr  */



int main(int argc, char *argv[])
{
    struct hostent *hp, *gethostbyname();
    struct sockaddr_in addr;

    int socket_desc;
    struct sockaddr_in server;
    char* message;
    char buf[1024];
    int n;

    if (argc != 3)
    {
        printf("Numero incorrecto de argumentos de linea de comando");
        return 1;
    }

    char *servidor = argv[1];
    int puerto = atoi(argv[2]);

    printf("El servidor es: %s \n", servidor);
    printf("El puerto seleccionado es: %i \n", puerto);


    // Primer paso, crear socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
	{
		puts("No se pudo crear el socket");
        return 1;
	}

    // Segundo paso, ingresar servidor

    if  ( (hp = gethostbyname(argv[1])) == NULL ) {
        addr.sin_addr.s_addr = inet_addr(argv[1]);
        if ((hp = gethostbyaddr((void *)&addr.sin_addr.s_addr,
            sizeof(addr.sin_addr.s_addr),AF_INET)) == NULL) {
            fprintf(stderr, "Can't find host %s\n", argv[1]);
            exit(-1);
        }
    }

    //server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = hp->h_addrtype;
	server.sin_port = htons(puerto);

    // Tercer paso, conectar socket a servidor
    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
		puts("connect error");
		return 1;
	}

	puts("Connected");

    // Ya estamos listos para enviar información
    puts("Ingrese a para sumar un voto a A");
    puts("Ingrese b para sumar un voto a B");
    puts("Ingrese f para terminar el conteo");

    while(1){
        n = read(0, buf, sizeof(buf));
        if (n == -1){
            puts("Error al leer input del usuario");
            return 1;
        } 

        if(strncmp(buf, "a", 1) == 0){
            if( send(socket_desc , buf , strlen(buf) , 0) < 0)
	        {
		        puts("Send failed");
                return 1;
	        }
            puts("Se envia voto para A");
        }

        if(strncmp(buf, "b", 1) == 0){
            if( send(socket_desc , buf , strlen(buf) , 0) < 0)
	        {
		        puts("Send failed");
                return 1;
	        }
            puts("Se envia voto para B");
        }

        if(strncmp(buf, "f", 1) == 0){
            puts("Se termina el conteo de votos");
            return 0;
        }
    }
    
    return 0;
}