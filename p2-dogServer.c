#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/stat.h>

#define PORT 3535
#define BACKLOG 2
#define MAX_CLIENTS 32

#define NOMBRE_SIZE 32
#define TIPO_SIZE 32
#define RAZA_SIZE 16

#define HASH_SIZE 10007 
#define MOD 256

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

struct logData
{
        unsigned char date [NOMBRE_SIZE];
        unsigned char ip [NOMBRE_SIZE];
        unsigned char operation [NOMBRE_SIZE];
        unsigned char cause[NOMBRE_SIZE];
};


void initPointers();
void loadHeads();
void initServer();
void saveHeads();


void enterPet();
void seePet();
void deletePet();
void searchPet();

int * lastID;
int fd, fdc[MAX_CLIENTS];

unsigned char operation [NOMBRE_SIZE];
unsigned char cause [NOMBRE_SIZE];


int getHash();
int countRegisters();
void saveDog();
void generateHc();

void getMascota();
void SendMascota();
int RegisterFromClient();
void SendConfirmation();
void logOperation();

struct sockaddr_in server, client[MAX_CLIENTS];
struct in_addr ipServer;

sem_t *semaphore;

//Carga el menu
int menu( int clientId )
{
	//Se recibe por el cliente un entero que corresponde a la funcionalidad escogida
	short option;
	
	int r = recv(fdc[clientId], &option, sizeof(option),0);
	if(r == 0)
	{
		perror("error por el connect");
		exit(-1);
	}

	switch(option)
	{
		case 1:
			enterPet(clientId);
			break;
		case 2:
			seePet(clientId);
			break;
		case 3:
			deletePet(clientId);
			break;
		case 4:
			searchPet(clientId);
			break;
		case 5:
			return 0; 
		default:
		printf("ingrese una opción valida\n");
	}	
	saveHeads();
	return 1;
}

pthread_t idThread[MAX_CLIENTS];
int arg[MAX_CLIENTS];

void *run( void * ap )
{
	int clientId = *(int*)ap;
	while(menu(clientId));
	pthread_join(idThread[clientId], NULL);
	close(fdc[clientId]);
	fdc[clientId] = 0;
}

int main()
{

	initPointers();
	loadHeads();
	initServer();

	for( int i = 0; i < MAX_CLIENTS; ++ i )
		arg[i] = i;

	semaphore = sem_open("semaphore_name",O_CREAT, NULL, MAX_CLIENTS);

	while( 1 ) 
	{
		
		int idx = 0;

		while( idx < MAX_CLIENTS && fdc[idx] ) ++idx;

		if( idx == MAX_CLIENTS ) 
		{
			perror("Limite de clientes superado");
			exit(-1);

		}	

		socklen_t tamaClient = 0;	
		fdc[idx] = accept(fd, (struct sockaddr *)&client[idx], &tamaClient);
		

		if( fdc[idx] == -1 )
		{
			perror("error en el accept");
			exit(-1);
		}

		
		int error = pthread_create(&idThread[idx], NULL, run, (void*)&arg[idx]);
		if (error != 0)
		{
			perror("error generando hilos");
			exit(-1);
		}

		sem_close(semaphore);
		sem_unlink("semaphore_name");

	}

	return 0;
}


void enterPet( int clientId )
{
	struct dogType * mascota;
	mascota = ( struct  dogType *) malloc( sizeof ( struct dogType ) );
	if( mascota == NULL )
	{
		perror("error en el malloc de la mascota");
		exit( -1 );
	}

	int r = recv(fdc[clientId], mascota, sizeof(struct dogType),0);
	
	if( r != sizeof(struct dogType) )
	{
		perror("Error reciviendo mascota del cliente");
		exit(-1);
	}
	sem_wait(semaphore);
	
	int hash = getHash( mascota -> nombre );
	mascota -> idPrev = lastID[hash];
	lastID[hash] = countRegisters();

	saveDog( mascota );

	fflush(stdout);
	sleep(1);
	sem_post(semaphore);

	SendConfirmation(1, clientId);	

    	memset( operation, 0, NOMBRE_SIZE * sizeof ( unsigned char ) );
    	strcat( operation, "inserción");

    	memset( cause, 0, NOMBRE_SIZE * sizeof ( unsigned char ) );
    	strcat( cause, mascota->nombre); 

    	free(mascota);
    	logOperation(operation,cause,clientId);
}


void seePet( int clientId )
{
	sem_wait(semaphore);
	int hcConfirmation;
	int registerId = RegisterFromClient( clientId );
	
	struct dogType * mascota;
	mascota = ( struct  dogType *) malloc( sizeof ( struct dogType ) );	
	if( mascota == NULL )
	{
		perror("error en el malloc de la mascota");
		exit( -1 );
	}

	getMascota(registerId, mascota);
	SendMascota(mascota, clientId );

	unsigned char hcName [22];//hc es historia clílica
	memset( hcName,0, sizeof (hcName));
	unsigned char hcFile [22];
	memset( hcFile,0, sizeof (hcFile));

	sprintf(hcName, "%d", registerId + 1);
	strcat(hcName,"hc.txt");

	if( access( hcName, F_OK ) == -1 )
       	{
		generateHc(hcName, mascota);
	}	

	strcat(hcFile,"gedit ");
	strcat(hcFile, hcName);


	int r = send(fdc[clientId], hcFile, sizeof(hcFile), 0);
	if( r != sizeof(hcFile) )
	{
		perror("Error enviando comando historia clinica");
		exit(-1);
	}

	r = recv(fdc[clientId], &hcConfirmation,sizeof(int),0);
	if( r != sizeof(registerId))
	{
		perror("error recibiendo id del cliente");
		exit(-1);
	}

	fflush(stdout);
	sleep(1);
	sem_post(semaphore);

	free(mascota);

	memset( operation, 0, NOMBRE_SIZE * sizeof ( unsigned char ) );
	strcat( operation, "lectura");

 	memset( cause, 0, NOMBRE_SIZE * sizeof ( unsigned char ) );
	sprintf( cause, "%d",registerId);

	logOperation(operation,cause,clientId);

}



void deletePet(int clientId )
{
	sem_wait(semaphore);

	int registerId = RegisterFromClient( clientId );
		
	struct dogType * mascotaFinal;
	mascotaFinal = ( struct  dogType *) malloc( sizeof ( struct dogType ) );	
	if( mascotaFinal == NULL )
	{
		perror("error en el malloc de la mascota");
		exit( -1 );
	}
	getMascota( countRegisters()-1, mascotaFinal );

	struct dogType * mascotaDelete;
	mascotaDelete = ( struct  dogType *) malloc( sizeof ( struct dogType ) );	
	if( mascotaDelete == NULL )
	{
		perror("error en el malloc de la mascota");
		exit( -1 );
	}
	getMascota( registerId, mascotaDelete );

	// a partir de este punto comienza lo feo	
	int hashF, hashD;
	hashF = getHash( mascotaFinal -> nombre );
	hashD = getHash( mascotaDelete -> nombre );

	struct dogType * tmp;
	tmp = ( struct  dogType *) malloc( sizeof ( struct dogType ) );	
	if( tmp  == NULL )
	{
		perror("error en el malloc de la mascota");
		exit( -1 );
	}
	
	int id, idAnt;
	id = -1;
	getMascota( lastID[hashF], tmp );
	while( 1 )
	{
		idAnt = tmp -> idPrev;
		if( tmp -> idPrev <= registerId )
			break;
		id = idAnt;
		getMascota( tmp -> idPrev, tmp );
	}	
	
	// id guarda el indice del primer elemento que pertenece a la lista de HashFinal y que se encuentra despues del elemento a ser borrado
	// si este elemento no existe se id vale -1 ( cuando la posicion borrada es la ultima )
	// para este elemento su prevId se modificara ya que antes la lista era de la forma:  ..... idAnt, id, ..... lastID[hashF]
	// ahora la lista pasa a ser de la forma: ..... id Prev, ultimaMascota(posicion eliminada), id, ......

	if( id != -1 )	
	{
		getMascota( id, tmp );
		tmp -> idPrev = registerId; // ahora tmp guarda la data completa de dicho elemento
	}
	
	// el head de la lista HashFinal, se re direcciona al que iba antes del ultimo elemento       
	lastID[hashF] = mascotaFinal -> idPrev;

	//ahora posicionando a la mascota final en la posicion eliminada(registerId) se apunta como elemento anterior a idPrev 
	mascotaFinal -> idPrev = idAnt;	

	struct dogType * tmp2;
	tmp2 = ( struct  dogType *) malloc( sizeof ( struct dogType ) );	
	if( tmp2  == NULL )
	{
		perror("error en el malloc de la mascota");
		exit( -1 );
	}
	
	int id2;
	id2 = -1;

	if( hashF != hashD )
	{	
		getMascota( lastID[hashD], tmp2 );
		while( 1 )
		{
			if( tmp2 -> idPrev <= registerId )
				break;
			id2 = tmp2 -> idPrev;
			getMascota( tmp2 -> idPrev, tmp2 );
		}	
		
		// id2 guarda el indice del primer elemento que apunta al elemento a ser borrado o -1 si el elemento borrado es el ultimo de su lista
		// la lista hashDelete era de la forma: ......( mascotaDelete -> prevId ), registerId, id2, .....
		// ahora es de la forma: ...... ( mascotaDelete -> prevId ), id2, .....
		
				
		if( id2 != -1 )	
		{
			getMascota( id2, tmp2 );
			tmp2 -> idPrev = mascotaDelete -> idPrev; // ahora tmp guarda la data completa del elemento id2
		}
		else
		{
			//el elemento a ser borrado es el ultimo de su lista, por tanto
			lastID[hashD] = mascotaDelete -> idPrev;
		}
	}
	else
	{
		//note que la informacion de mascotaFinal -> prevId es registerId, hay que arreglarlo porque se apuntaria a si misma
		mascotaFinal -> idPrev = mascotaDelete -> idPrev;
	}

	// aqui acaba lo feo :D

	//Re escritura del archivo	
	
	int status;
	
	FILE * dbTmp;
	int check;
	dbTmp = fopen("dbTmp.dat", "a+");
	
	int i, tot;
	i = 0, tot = countRegisters() - 1;

	for( i = 0; i < tot; ++ i )
	{
		if( i == id ){
			check = fwrite ( tmp, sizeof( struct dogType ), 1, dbTmp );	
			if (check==0)
			{
				perror("error escritura dbTmp");
				exit(-1);
			}
		} 
		else if( i == id2 ){
			check = fwrite ( tmp2, sizeof( struct dogType ), 1, dbTmp );
			if (check==0)
			{
				perror("error escritura dbTmp");
				exit(-1);
			}	
		}
		else if( i == registerId ){
			check = fwrite ( mascotaFinal, sizeof( struct dogType ), 1, dbTmp );
			if (check==0)
			{
				perror("error escritura dbTmp");
				exit(-1);
			}
		} 
		else
		{
			getMascota( i, mascotaDelete );
			check  = fwrite( mascotaDelete, sizeof ( struct dogType ), 1, dbTmp ); 
			if (check==0)
			{
				perror("error escritura dbTmp");
				exit(-1);
			}
		}
	}
	fclose(dbTmp); 

	status = remove( "dataDogs.dat" );	
	if( status != 0 )
	{
		perror( "Error eliminando la base de datos" );
		exit( -1 );
	}

	status = rename( "dbTmp.dat", "dataDogs.dat" );
	if( status != 0 )
	{
		perror( "Error en el renombramiento de la base de datos" );
		exit( -1 );
	}

	//Re escritura de historias clinicas
	

	// Primero se elimina la historia clinica de la mascota registerId
	unsigned char hcNameDelete [22], hcNameLast [22];
	memset( hcNameDelete,0, sizeof (hcNameDelete));
	memset( hcNameLast, 0, sizeof (hcNameLast) );

	sprintf(hcNameDelete, "%d", registerId + 1);
	strcat(hcNameDelete,"hc");
	strcat(hcNameDelete,".txt");

		
	sprintf(hcNameLast, "%d", countRegisters() + 1 );
	strcat(hcNameLast,"hc");
	strcat(hcNameLast,".txt");

	if( access( hcNameDelete, F_OK ) != -1 ) { 
		status = remove( hcNameDelete );	
		if( status != 0 )
		{
			perror( "Error eliminando historia clinica" );
			exit( -1 );

		}
	}

	fflush(stdout);
	sleep(1);
	sem_post(semaphore);	

	//Ahora se renombra la historia clinica del ultimo	
	
	if( access( hcNameLast, F_OK ) != -1 ) {
		status = rename( hcNameLast, hcNameDelete );
		if( status != 0 )
		{
			perror( "Error en el renombramiento de la historio clinica" );
			exit( -1 );
		}
		
	}


    	memset( operation, 0, NOMBRE_SIZE * sizeof ( unsigned char ) );
    	strcat( operation, "borrado");

    	memset( cause, 0, NOMBRE_SIZE * sizeof ( unsigned char ) );
    	sprintf( cause,"%d", registerId);

	free(mascotaFinal);
	free(mascotaDelete);
	free(tmp);
	free(tmp2);

	SendConfirmation(1, clientId );

    	logOperation(operation,cause,clientId);
}

void searchPet( int clientId )
{
	unsigned char buff[NOMBRE_SIZE];

	int r = recv(fdc[clientId], buff, sizeof( buff ), 0 );
	if( r != sizeof ( buff ) )
	{
		perror("Error recibiendo nombre a buscar");
		exit(-1);
	}

	int hash, currId, i, equal, a, b;
	hash = getHash( buff );
	
	currId = lastID[hash];

	struct dogType * mascota;
	mascota = ( struct  dogType *) malloc( sizeof ( struct dogType ) );	
	if( mascota == NULL )
	{
		perror("error en el malloc de la mascota");
		exit( -1 );
	}
	
	while( currId != -1 )
	{
		getMascota( currId, mascota );
	
		equal = 1;

		for( i = 0; i < NOMBRE_SIZE; ++i )
		{

			a = (int) mascota->nombre[i];
        	        b = (int) buff[i];
			if( a >= 'A' && a <= 'Z' ) a = 'a' + ( a - 'A' );
                	if( b >= 'A' && b <= 'Z' ) b = 'a' + ( b - 'A' );
		
                	if( a != b ) 
                		equal = 0;
		}
		if( equal )
		{
			SendConfirmation(currId+1, clientId );
			SendMascota( mascota, clientId );	
		}
		currId = mascota -> idPrev;
	}

	free(mascota);
	SendConfirmation(-1, clientId );

	memset( operation, 0, NOMBRE_SIZE * sizeof ( unsigned char ) );
    	strcat( operation, "búsqueda");

    	memset( cause, 0, NOMBRE_SIZE * sizeof ( unsigned char ) );
    	strcat( cause, buff);

    	logOperation(operation,cause,clientId);
}

void SendConfirmation(int confirmation, int clientId ) 
{
	int r = send(fdc[clientId], &confirmation, sizeof(confirmation), 0);
	if( r != sizeof(confirmation) )
	{
		perror("Error enviando confirmacion al cliente");
		exit(-1);
	}
}

int RegisterFromClient( int clientId )
{
	int auxCount = countRegisters();
	int r = send(fdc[clientId], &auxCount, sizeof(auxCount), 0);
	
	if( r != sizeof( auxCount) )
	{
		perror("Error enviando el total de registros");
		exit(-1);
	}


	int registerId;
	
	r = recv(fdc[clientId], &registerId,sizeof(registerId),0);
	if( r != sizeof(registerId))
	{
		perror("error recibiendo id del cliente");
		exit(-1);
	}
	return --registerId;
}

void getMascota( int idx, struct dogType * mascota )
{
	FILE *g;
	int  check, size;
	g = fopen("dataDogs.dat","r");
	if(g == NULL){
		perror("error abriendo archivo (getMascota)");
		exit(-1);
	}

	check = fseek(g, idx * sizeof( struct dogType ), SEEK_SET);
	if (check == -1)
	{
		perror("error seek get Mascota");
		exit( -1 );
	}


	check = fread(mascota, sizeof( struct dogType ),1,g );
	if ( check == 0 )
	{
		perror("error en el read del archivo (getMascota)");
		exit(-1);
	}		
	fclose( g );
}

void SendMascota( struct dogType * mascota, int clientId )
{
	int r = send(fdc[clientId], mascota, sizeof(struct dogType), 0);
	if( r != sizeof(struct dogType) )
	{
		perror("Error enviando mascota el cliente");
		exit(-1);
	}
}

void initPointers()
{
	lastID = ( int * ) malloc ( HASH_SIZE * sizeof ( int ) );
	if( lastID == NULL )
	{
		perror("error en el malloc lastID" );
		exit( -1 );
	}
        memset( lastID, -1, HASH_SIZE * sizeof ( int ) );
}

void loadHeads()
{
	FILE * points;
 	int check;

	points = fopen("dataPointers.dat", "r");
	if(points == NULL)
	{
                perror("error cargando apuntadores");
                exit(-1);
        }

	check = fread(lastID, HASH_SIZE * sizeof(int), 1, points );
	if( check == 0 )
	{
		perror("error en la lectura de apuntadores");
		exit( -1 );
	}

	fclose( points );
}

void initServer()
{
	fd = socket(AF_INET, SOCK_STREAM,0);
	
	if(fd == -1)
	{
		perror("error generando socket del servidor");
		exit(-1);
	}
	server.sin_family = AF_INET;
	server.sin_port = htons(PORT);
	server.sin_addr.s_addr = INADDR_ANY;
	ipServer = server.sin_addr;
	bzero(server.sin_zero,8);

	size_t tama = sizeof(struct sockaddr_in);
	int r = bind(fd,(struct sockaddr*)&server,tama);
	if(r == -1)
	{
		perror("error en el bind");
		exit(-1);
	}
	r = listen(fd,BACKLOG);
	if(r == -1)
	{
		perror("error en el listen");
		exit(-1);
	}
}

void saveHeads()
{
        FILE *points;
        int  check;

        points = fopen("dataPointers.dat","w+");
        if(points == NULL)
	{
                perror("error generando apuntadores");
                exit(-1);
        }

        check = fwrite( lastID, HASH_SIZE * sizeof(int), 1, points );
        if( check == 0 )
	{
		perror("error en la escritura de apuntadores");	
		exit( -1 );
	}
	fclose(points);
}


int getHash( unsigned char * nombre )
{
        int hash, base, i, auxC;

        for( i = 0, hash = 0, base = 1; i < NOMBRE_SIZE; ++ i )
        {
                auxC = (int) nombre[i];
                if( auxC >= 'A' && auxC <= 'Z' ) auxC = 'a' + ( auxC - 'A' );

                hash += auxC * base;

                base *= MOD;
                base %= HASH_SIZE;

                hash %= HASH_SIZE;
        }

        return hash;
}

int countRegisters(){
	FILE *g;
	int  check, size;
	g = fopen("dataDogs.dat","r");
	if(g == NULL){
		perror("error abriendo archivo (size)");
		exit(-1);
	}

	check = fseek(g,0L, SEEK_END);
	if (check == -1)
	{
		perror("error seek size archivo");
		exit( -1 );
	}

	size = ftell(g);
	if (size == -1)
	{
		perror("error tell size archivo");
		exit(-1);
	}

	fclose(g);

	return size / sizeof ( struct dogType );
}

void saveDog ( void *ap )
{
	FILE *f;
	f = fopen("dataDogs.dat", "a+");
	if( f == NULL )
	{
		perror("error abriendo archivo principal");
		exit( -1 );
	}

	struct dogType * dato;
	dato = (struct dogType *) ap;

	int r;
	r = fseek(f,0L, SEEK_END);
	if( r == -1 )
	{
		perror("error en la escritura de la mascota(seek)");
		exit( -1 );
	}
	r = fwrite( dato, sizeof( struct dogType ), 1, f );
	if( r == 0 )
	{
		perror("error en la escritura en el archivo principal");
		exit( -1 );
	}
	fclose( f );
}

void generateHc(char* hcName, struct dogType* mascota){
	FILE *hc = fopen(hcName,"w+");
	int check;
	check = fprintf(hc,"NOMBRE: %s\n", mascota->nombre);
	if (check < 0){
		perror("error generando historia clinica");
		exit(-1);
	}
	check = fprintf(hc,"TIPO: %s\n", mascota->tipo);
	if (check < 0){
		perror("error generando historia clinica");
		exit(-1);
	}
	check = fprintf(hc,"EDAD: %d\n", mascota->edad);
	if (check < 0){
		perror("error generando historia clinica");
		exit(-1);
	}
	check = fprintf(hc,"RAZA: %s\n", mascota->raza);
	if (check < 0){
		perror("error generando historia clinica");
		exit(-1);
	}
	check = fprintf(hc,"ESTATURA: %d\n", mascota->estatura);
	if (check < 0){
		perror("error generando historia clinica");
		exit(-1);
	}
	check = fprintf(hc,"PESO: %f\n", mascota->peso);
	if (check < 0){
		perror("error generando historia clinica");
		exit(-1);
	}
	check = fprintf(hc,"SEXO: %s\n", &mascota->sexo);
	if (check < 0){
		perror("error generando historia clinica");
		exit(-1);
	}

	check = fprintf(hc,"Historia clinica: \n");
	if (check < 0){
		perror("error generando historia clinica");
		exit(-1);
	}

	fclose(hc);
}

void logOperation(unsigned char* operation, unsigned char* cause, int clientId ){
	

	struct logData * log;
        log = ( struct  logData *) malloc( sizeof ( struct logData ) );
        if( log == NULL )
        {
                perror("error en el malloc del log");
                exit( -1 );
        }

        time_t t = time(NULL);
        struct tm tm = *localtime(&t);
       
       	unsigned char actualTime [NOMBRE_SIZE];
        memset( actualTime, 0, NOMBRE_SIZE * sizeof ( unsigned char ) );
        unsigned char times [NOMBRE_SIZE];
        memset( times, 0, NOMBRE_SIZE * sizeof ( unsigned char ) );

        strcat(actualTime, "|");
        sprintf(times, "%d", tm.tm_year + 1900);
        strcat(actualTime, times);
        strcat(actualTime, "|");
        sprintf(times, "%d",tm.tm_mon + 1);
        strcat(actualTime, times);
        strcat(actualTime, "|");
        sprintf(times, "%d",tm.tm_mday);
        strcat(actualTime, times);
        strcat(actualTime, "|");
        sprintf(times, "%d",tm.tm_hour);
        strcat(actualTime, times);
        strcat(actualTime, "|");
        sprintf(times, "%d",tm.tm_min);
        strcat(actualTime, times);
        strcat(actualTime, "|");
        sprintf(times, "%d",tm.tm_sec);
        strcat(actualTime, times);
        strcat(actualTime, "|");
	
	memset( log->date, 0, NOMBRE_SIZE );
        strcpy(log->date, actualTime);


        char ipAddr[INET_ADDRSTRLEN];
	inet_ntop( AF_INET, &(client[clientId].sin_addr), ipAddr, INET_ADDRSTRLEN);
        
	memset(log->ip, 0, NOMBRE_SIZE);	

	strcpy(log->ip, ipAddr);

	memset(log->operation,0,NOMBRE_SIZE);
	memset(log->cause, 0, NOMBRE_SIZE);

        strcpy(log->operation, operation);
        strcpy(log->cause, cause);

        FILE * logFile = fopen("dataDogs.log","a+");
        if( logFile == NULL ){
		perror("error abriendo archivo log");
		exit( -1 );
	}

	int w = fwrite ( log, sizeof( struct logData ), 1, logFile );	
	if (w==0){
		perror("error escritura logData");
		exit(-1);
	}
	fclose(logFile);
	free(log);
}
