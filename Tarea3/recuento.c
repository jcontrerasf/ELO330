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

    int socket_desc;
    struct sockaddr_in server;
    char* message;
    char buf[1024];
    int n;

    if (argc != 2)
    {
        printf("Numero incorrecto de argumentos de linea de comando");
        return 1;
    }

    //char *servidor = argv[1];
    int puerto = atoi(argv[1]);

    //printf("El servidor es: %s \n", servidor);
    printf("El puerto seleccionado es: %i \n", puerto);


    // Primer paso, crear socket
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
	{
		puts("No se pudo crear el socket");
        return 1;
	}

    // Segundo paso, ingresar servidor
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_family = AF_INET;
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