#include<stdio.h>
#include<stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#define PORT 3535
#define BACKLOG 2
#define NOMBRE_SIZE 32
#define TIPO_SIZE 32
#define RAZA_SIZE 16

struct dogType
{
        unsigned char nombre [NOMBRE_SIZE];
        unsigned char tipo [TIPO_SIZE];
        int edad;
        unsigned char raza [RAZA_SIZE];
        int estatura;
        float peso;
        unsigned char sexo;

	int idPrev;
};

void imprimirMascota( struct dogType * mascota )
{

	printf("NOMBRE: %s\n", mascota -> nombre );
	printf("TIPO: %s\n", mascota -> tipo );
	printf("EDAD: %d\n", mascota -> edad );
	printf("RAZA: %s\n", mascota -> raza );
	printf("ESTATURA: %d\n", mascota -> estatura );
	printf("PESO: %f\n", mascota -> peso );
	printf("SEXO: %c\n", mascota -> sexo );
	printf("\n\n" );
}

int main(){
	struct dogType * mascota;
	mascota = ( struct  dogType *) malloc( sizeof ( struct dogType ) );	
	if( mascota == NULL )
	{
		perror("error en el malloc de la mascota");
		exit( -1 );
	}

	struct sockaddr_in server, client1;
	int fd = socket(AF_INET, SOCK_STREAM,0);
	
	if(fd == -1){
		perror("error generando socket");
		exit(-1);
	}
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = INADDR_ANY;
	bzero(server.sin_zero,8);

	size_t tama = sizeof(struct sockaddr_in);
	int r = bind(fd,(struct sockaddr*)&server,tama);
	if(r == -1){
		perror("error por el bind");
		exit(-1);
	}
	r = listen(fd,BACKLOG);
	if(r == -1){
		perror("error por el listen");
		exit(-1);
	}
	
	socklen_t  tamaClient = 0;	
	int fd1 = accept(fd, (struct sockaddr *)&client1, &tamaClient);
	
	if( fd1 == -1 ){
		perror("error en el accept");
		exit(-1);
	}

	r = recv(fd1, mascota,sizeof(struct dogType),0);
	if(r == 0){
		perror("error por el connect");
		exit(-1);
	}

	imprimirMascota(mascota);

	r = send(fd1,"Hola mundo", 10, 0);
	if( r == -0 ){
		perror("error en el send");
		exit(-1);
	}

	close(fd1);
	close(fd);
	return 0;
	
}

