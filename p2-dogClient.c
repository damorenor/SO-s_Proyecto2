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
int fd,r;
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
			r = send(fd,&option,sizeof(short), 0);
			if( r == 0 ){
				perror("error en el send");
				exit(-1);
			}
			enterPet();
			break;
		case 2:
			r = send(fd,&option,sizeof(short), 0);
			if( r == 0 ){
				perror("error en el send");
				exit(-1);
			}
			seePet();
			break;
		case 3:
			r = send(fd,&option,sizeof(short), 0);
			if( r == 0 ){
				perror("error en el send");
				exit(-1);
			}
			deletePet();
			break;
		case 4:
			r = send(fd,&option,sizeof(short), 0);
			if( r == 0 ){
				perror("error en el send");
				exit(-1);
			}
			searchPet();
			break;
		case 5:
			r = send(fd,&option,sizeof(short), 0);
			if( r == 0 ){
				perror("error en el send");
				exit(-1);
			}
			close(fd);
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

	r = connect(fd,(struct sockaddr *)&server,tama);
	if(r == -1){
		perror("error por el connect");
		exit(-1);
	}
	run();
}

void enterPet(){
	//Manda al server opciónh
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
	r = send(fd,mascota,sizeof(struct dogType), 0);
	if( r == 0 ){
		perror("error en el send");
		exit(-1);
	}
	
	//Recibe confirmación
	int check;
	r = recv(fd,&check,sizeof(int),0);
	if(r == 0){
		perror("error por el connect");
		exit(-1);
	}	

	if(check == 1){
		printf("\nEl registro de la mascota se ha ingresado satisfactoriamente\n");
	}

	free(mascota);

	printf("Presione Enter para continuar\n");
	char enter = 0;
	getchar();
	while (enter != '\r' && enter != '\n') { 
		enter = getchar();
	}
}

void seePet(){
	system("clear");
	printf("Numero de perros registrados: ");
	int totalRegister, check;
	struct dogType * mascota;
	mascota = ( struct  dogType *) malloc( sizeof ( struct dogType ) );	
	if( mascota == NULL )
	{
		perror("error en el malloc de la mascota");
		exit( -1 );
	}

	r = recv(fd,&totalRegister,sizeof(int),0);
	if(r == 0){
		perror("error por el connect");
		exit(-1);
	}
	printf("%d\n", totalRegister);

	printf("Ingrese el id de la mascota que desea ver:\n");
	int registerId;
	scanf("%d",&registerId);

	r = send(fd,&registerId,sizeof(int), 0);
	if( r == 0 ){
		perror("error en el send");
		exit(-1);
	}

	r = recv(fd,mascota,sizeof(struct dogType),0);
	if(r == 0){
		perror("error por el connect");
		exit(-1);
	}

	imprimirMascota(mascota);

	unsigned char hcfile[20];
	memset( hcfile, 0, sizeof hcfile);	
	r = recv(fd,hcfile,20,0);
	if(r == 0){
		perror("error por el connect");
		exit(-1);
	}	

	system(hcfile);

	free(mascota);

	printf("\nPresione Enter para continuar\n");
	char enter = 0;
	getchar();
	while (enter != '\r' && enter != '\n') { 
		enter = getchar();
	}
}

void deletePet(){
	system("clear");
	printf("Numero de perros registrados: ");
	int totalRegister, id_deleted,check;

	r = recv(fd,&totalRegister,sizeof(int),0);
	if(r == 0){
		perror("error por el connect");
		exit(-1);
	}
	printf("%d\n", totalRegister);

	printf("Ingrese el id de la mascota que desea eliminar:\n");
	scanf("%d",&id_deleted);

	r = send(fd,&id_deleted,sizeof(int), 0);
	if( r == 0 ){
		perror("error en el send");
		exit(-1);
	}

	r = recv(fd,&check,sizeof(int),0);
	if(r == 0){
		perror("error por el connect");
		exit(-1);
	}	

	if(check == 1){
		printf("\nEl registro de la mascota se ha eliminado satisfactoriamente\n");
	}

	printf("\nPresione Enter para continuar\n");
	char enter = 0;
	getchar();
	while (enter != '\r' && enter != '\n') { 
		enter = getchar();
	}
}

void searchPet(){
	struct dogType * mascota;
	mascota = ( struct  dogType *) malloc( sizeof ( struct dogType ) );	
	if( mascota == NULL )
	{
		perror("error en el malloc de la mascota");
		exit( -1 );
	}

	system("clear");
	printf("Ingrese el nombre de la mascota que quiere buscar:\n" );

	unsigned char buff[NOMBRE_SIZE];

	
	memset( buff, 0, sizeof buff );	
	scanf("%s", buff );
	printf("\n");

	r = send(fd,buff,NOMBRE_SIZE, 0);
	if( r == 0 ){
		perror("error en el send");
		exit(-1);
	}

	int id_searched;
	while(1){
		r = recv(fd,&id_searched,sizeof(int),0);
		if(r == 0){
			perror("error por el connect");
			exit(-1);
		}

		if(id_searched == -1){
			break;
		}else{
			r = recv(fd,mascota,sizeof(struct dogType),0);
			if(r == 0){
				perror("error por el connect");
				exit(-1);
			}
			imprimirMascota(mascota);
		}
	}

	free(mascota);

	printf("\nPresione Enter para continuar\n");
	char enter = 0;
	getchar();
	while (enter != '\r' && enter != '\n') { 
		enter = getchar();
	}
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
