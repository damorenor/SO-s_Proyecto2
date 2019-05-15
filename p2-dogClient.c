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

void enterPet();
void seePet();
void deletePet();
void searchPet();

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
			seePet();
			break;
		case 3:
			deletePet();
			break;
		case 4:
			searchPet();
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
	run();
}

void enterPet(){
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
	
	//Manda la mascota

	//Recibe confirmación

	//exit() bla bla bla
	free(mascota);
}