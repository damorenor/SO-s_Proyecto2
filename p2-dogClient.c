#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<string.h>
#include<arpa/inet.h>
#include<unistd.h>
#define PORT 3535
#define BACKLOG 2
#define NOMBRE_SIZE 32
#define TIPO_SIZE 32
#define RAZA_SIZE 16

struct sockaddr_in server, client1;
int fd;
size_t tama;

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

void enterPet();
void seePet();
void deletePet();
void searchPet();

void imprimirMascota();

void menu(){
	//Se imprime en consola el menú de usuario
	system("clear");
	printf("\t\t\tVET CLINIC :3\n"
			"est.2019\n\n\n"
			"\t1.\tIngresar Registro\n"
			"\t2.\tVer Registro\t\n"
			"\t3.\tBorrar Registro\n"
			"\t4.\tBuscar Registro\n"
			"\t5.\tSalir\n"
		);
	//Se recibe por teclado un entero que corresponde a la funcionalidad escogida
	short option;
	scanf("%hd",&option);
	switch(option){
		case 1:
			enterPet();
			break;
		case 2:
			//seePet();
			break;
		case 3:
			//deletePet();
			break;
		case 4:
			//searchPet();
			break;
		case 5:
			//Agregar cierres conexión bla bla
			exit( 0 );

		default:
		printf("ingrese una opción valida\n");
	}	
}

void run(){
	while(1){
		menu();

	}
}
int main(){
	
	fd = socket(AF_INET, SOCK_STREAM,0);
	
	if(fd == -1){
		perror("error generando socket");
		exit(-1);
	}
	
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = inet_addr("127.0.0.1");
	bzero(server.sin_zero,8);
	tama = sizeof(struct sockaddr_in);
	run();
}

void enterPet(){
	char buffer[20];
	memset(buffer,0, sizeof(buffer) );
	//Manda al server opción
	system("clear");
	printf("Ingrese porfavor los datos de la mascota:\n\n");

	struct dogType * mascota;
	mascota = ( struct  dogType *) malloc( sizeof ( struct dogType ) );	
	if( mascota == NULL )
	{
		perror("error en el malloc de la mascota");
		exit( -1 );
	}
	
	memset( mascota->nombre, 0, NOMBRE_SIZE * sizeof ( unsigned char ) );
	printf("ingrese nombre:");
	scanf("%s",mascota->nombre);
	printf("\ningrese tipo:");
	scanf("%s",mascota->tipo);
	printf("\ningrese edad:");
	scanf("%d",&mascota->edad);
	printf("\ningrese raza:");
	scanf("%s",mascota->raza);
	printf("\ningrese estatura:");
	scanf("%d",&mascota->estatura);
	printf("\ningrese peso:");
	scanf("%f",&mascota->peso);
	printf("\ningrese sexo:");
	scanf("%s",&mascota->sexo);
	printf("\n\n" );
	

	imprimirMascota(mascota);	
	//Manda la mascota
	int r = connect(fd,(struct sockaddr *)&server,tama);
	if(r == -1){
		perror("error por el connect");
		exit(-1);
	}

	r = send(fd,mascota,sizeof(struct dogType), 0);
	if( r == 0 ){
		perror("error en el send");
		exit(-1);
	}	
	
	printf("%zu\n%d\n", sizeof(mascota),r);
	//Recibe confirmación
	r = recv(fd, buffer,10,0);
	if(r == 0){
		perror("error por el connect");
		exit(-1);
	}
	printf("%s\n", buffer);
	//exit() bla bla bla
	printf("\ncambios en la historia clinica realizados ingrese un caracter para continuar");
	char end;
	scanf("%s",&end);

	free(mascota);
}

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