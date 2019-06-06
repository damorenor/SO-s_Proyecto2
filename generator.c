//damorenor + joacarrilloco
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define REGISTER_SIZE ( (int) 1e7 ) 
#define NOMBRE_SIZE 32
#define TIPO_SIZE 32
#define RAZA_SIZE 16
#define TOTAL_NAMES 1716
#define TOTAL_BREEDS 13

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


FILE *f;

unsigned char ** mainNames;
unsigned char ** mainBreeds;
int * lastID; 

void initPointers();
void nameArrayGenerator();
void breedArrayGenerator();
int getHash( unsigned char * nombre );
void saveDog( void *ap );
void saveHeads();


int main()
{	
	srand( time( NULL ) );

	initPointers();
	nameArrayGenerator();
	breedArrayGenerator();


	struct dogType * perro;
	perro = ( struct  dogType *) malloc( sizeof ( struct dogType ) );	
	if( perro == NULL )
	{
		perror("error en el malloc");
		exit( -1 );
	}

	int i, hash;	
	for( i = 0; i < REGISTER_SIZE; ++ i )
	{

		memset( perro -> nombre, 0, sizeof ( perro -> nombre ) );

		strcpy( perro -> nombre, mainNames[ rand()%TOTAL_NAMES] );
		strcpy( perro -> tipo, "perro" );		
		perro -> edad = rand() % 18; 	
		strcpy( perro -> raza, mainBreeds[ rand()%TOTAL_BREEDS ] );
		perro -> estatura  = 50 + rand() % 51; 
		perro -> peso = 10.0 + 20.0 * ( (float) rand() / RAND_MAX );	
		perro -> sexo = ( rand() % 2 ) ? 'H' : 'M' ;


		hash = getHash( perro -> nombre );
		perro -> idPrev = lastID[hash];
		lastID[hash] = i;


		saveDog( perro );	
	}	

	saveHeads();

	fclose( f );
	return 0;
}

void saveHeads(){
	FILE *points;
	int  check;

	points = fopen("dataPointers.dat","w+");
	if(points == NULL){
		perror("error generando apuntadores");
		exit(-1);
	}
	
	fwrite( lastID, HASH_SIZE * sizeof(int), 1, points );
	fclose(points);
}

void initPointers()
{
	f = fopen("dataDogs.dat", "w+");
	if( f == NULL )
	{
		perror("error abriendo archivo principal");
		exit( -1 );
	}

	mainNames = ( unsigned char ** ) malloc( TOTAL_NAMES * sizeof ( unsigned char * ) );
	if( mainNames == NULL )
	{
		perror("error en el malloc de mainNames");
		exit(-1);
	}
	int i;
	for( i = 0; i < TOTAL_NAMES; ++ i )
	{
		mainNames[i] = (unsigned char * ) malloc ( NOMBRE_SIZE * sizeof ( unsigned char ) );
		if( mainNames[i] == NULL )
		{
			perror("error en el malloc de mainNames");
			exit(-1);
		}
		memset( mainNames[i], 0, NOMBRE_SIZE * sizeof ( unsigned char ) );
	}

	mainBreeds = ( unsigned char ** ) malloc( TOTAL_BREEDS * sizeof ( unsigned char * ) );
	if( mainBreeds == NULL )
	{
		perror("error en el malloc de mainBreeds");
		exit(-1);
	}
	
	for( i = 0; i < TOTAL_BREEDS; ++ i )
	{
		mainBreeds[i] = (unsigned char * ) malloc ( RAZA_SIZE * sizeof ( unsigned char ) );
		if( mainBreeds[i] == NULL )
		{
			perror("error en el malloc de mainBreeds");
			exit(-1);
		}
		memset( mainBreeds[i], 0, RAZA_SIZE * sizeof ( unsigned char ) );
	}

        lastID = ( int * ) malloc ( HASH_SIZE * sizeof ( int ) );
	if( lastID == NULL )
	{
		perror("error en el malloc lastID" );
		exit( -1 );
	}
	memset( lastID, -1, HASH_SIZE * sizeof ( int ) );
	
}


void nameArrayGenerator(){
	FILE *names;
	names = fopen("dataNames.dat","r");
	if(names == NULL){
		perror("error open de dataNames");
		exit(-1);
	}
	int check;
	unsigned char buffer[NOMBRE_SIZE];
	for(int i=0;i<TOTAL_NAMES;i++){
		check = fseek(names,sizeof(buffer)*i,SEEK_SET);
		if(check == -1){
			perror("error seek dataNames");
			exit(-1);
		}

		check = fread(&buffer,sizeof(buffer),1,names);
		if(check == 0){
			perror("error lectura de dataNames");
			exit(-1);
		}

		strcpy(mainNames[i], buffer);
	}
	fclose(names);
}


void breedArrayGenerator(){
        FILE *breeds;
        breeds = fopen("dataBreeds.dat","r");
        if(breeds == NULL){
                perror("error open de dataBreeds");
                exit(-1);
        }
        int check;
        unsigned char buffer[RAZA_SIZE];
        for(int i=0;i<TOTAL_BREEDS;i++){
                check = fseek(breeds,sizeof(buffer)*i,SEEK_SET);
                if(check == -1){
                        perror("error seek dataBreeds");
                        exit(-1);
                }

                check = fread(&buffer,sizeof(buffer),1,breeds);
                if(check == 0){
                        perror("error lectura de dataBreeds");
                        exit(-1);
                }

                strcpy(mainBreeds[i], buffer);
        }
        fclose(breeds);
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


void saveDog ( void *ap )
{
	struct dogType * dato;
	dato = (struct dogType *) ap;

	int r;
	r = fwrite( dato, sizeof( struct dogType ), 1, f );
	if( r == 0 )
	{
		perror("error en la escritura en el archivo principal");
		exit( -1 );
	}
}


