#include <iostream>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

const int MEM_KEY = 4096;
const int SEM_KEY = 4096;
const int BUFOR_SIZE = 100;
#define write_ptr buf_ptr[100]
#define read_ptr buf_ptr[101]

using namespace std;

enum { EMPTY, FULL, MUTEX,CONS_MUTEX, PROD_MUTEX, };

//**********************************************************************

struct sembuf semaphoreControl = {0, 0, 0};

void up(int semaphoreType, int semaphoreID) 	//V()
{
	semaphoreControl.sem_num = semaphoreType;
	semaphoreControl.sem_op = 1; 			//dodaje do semafora wartosc sem_num
	if(semop(semaphoreID, &semaphoreControl, 1) == -1)
		cout<<"Blad funkcji up semafora!"<<endl;
}

void down(int semaphoreType, int semaphoreID) 			//P()
{
		semaphoreControl.sem_num = semaphoreType;
		semaphoreControl.sem_op = -1;		//odejmuje od semafora wartosc sem_num
		if(semop(semaphoreID, &semaphoreControl, 1) == -1)
			cout<<"Blad funkcji down semafora!"<<endl;
}

//**********************************************************************


//argv[1] = 1 - tworzenie pamieci wspoldzielonej
//argv[1] = 2 - konsument, argv[2] - liczba elementow do czytania
//argv[1] = 3 - producent, argv[2] - liczba elementow do wyprodukowania
//producent wrzuca do bufora losowa wartosc

int main(int argc, char **argv)
{
	
	if(argc != 3)
		return -1;
		
	int sharedMemoryId, semaphoreId;
	int *buf_ptr;
	
	if(atoi(argv[1]) == 1)
	{
		cout<<"Tworzenie pamieci wspoldzielonej.\n";

		sharedMemoryId = shmget(MEM_KEY, (BUFOR_SIZE+2)*sizeof(int) , 0777 | IPC_CREAT | IPC_EXCL);
		
		if(sharedMemoryId == -1)
		{
			cout<<"Blad przy tworzeniu pamieci wspoldzielonej.\n";
			return -1;
		}
		
		buf_ptr = (int *)shmat(sharedMemoryId, NULL, 0);
		buf_ptr[101] = 0; //read_ptr = 0
		buf_ptr[100] = 0; //write_ptr = 0
		
		cout<<"Tworzenie pamieci semaforow.\n";
		if ((semaphoreId = semget(SEM_KEY, 5 , 0777 | IPC_CREAT | IPC_EXCL)) == -1) 
		{
        		cout <<"Blad przy tworzeniu semaforow.\n";
        		return -1;
		}
		
		//EMPTY - wartosc BUFOR_SIZE
		//FULL - wartosc 0
		//MUTEX - wartosc 1
		//PROD_MUTEX - wartosc 1
		//CONS_MUTEX - wartosc 1
		if (semctl(semaphoreId, EMPTY , SETVAL, BUFOR_SIZE) == -1  || 
			semctl(semaphoreId, FULL , SETVAL, 0) == -1 || 
			semctl(semaphoreId, MUTEX , SETVAL, 1) == -1 || 
			semctl(semaphoreId, CONS_MUTEX , SETVAL, 1) == -1 || 
			semctl(semaphoreId, PROD_MUTEX , SETVAL, 1) == -1)
		{
			semctl(SEM_KEY, 0, IPC_RMID); 									
			cout <<"Blad przy inicjalizacji semaforow.\n";
			return -1;
		}
		cout<<"Tworzenie pamieci wspoldzielonej i semaforow zakonczone powodzeniem\n";
	}
	
	if(atoi(argv[1]) == 2 || atoi(argv[1]) == 3)	//przypisanie procesu do utworzonej wczesniej pamieci wspoldzielonej
	{
		if ((sharedMemoryId = shmget(MEM_KEY, (BUFOR_SIZE+2)*sizeof(int) , 0)) == -1)
		{
			cout << "Nie mozna dostac sie do pamieci wspoldzielonej";
			return -1;
		}

		
		if ((semaphoreId = semget(SEM_KEY, 5 , 0)) == -1)
		{
			cout << "Nie mozna dostac sie do semaforow";
			return -1;
		}
	}
	
	
	if(atoi(argv[1]) == 2)			//konsument
	{
		int amountOfItems = atoi(argv[2]);
		int value;
		
		if(amountOfItems < 0)
		{
			cout<<"Niepoprawny argument\n";
			return -1;
		}
		
		down(CONS_MUTEX, semaphoreId);
		buf_ptr = (int *)shmat(sharedMemoryId, NULL, 0);
		
		while( amountOfItems > 0 )
		{
			
			down(FULL, semaphoreId);
			down(MUTEX, semaphoreId);
			cout<<"Z adresu bufora nr: "<<read_ptr<<endl;
			value = buf_ptr[read_ptr];
			read_ptr = (read_ptr + 1)%BUFOR_SIZE;
			cout << "Konsument: " << getpid() << " odczytal element: " << value <<endl;

			sleep (1);
			up(MUTEX, semaphoreId);
			up(EMPTY, semaphoreId);
			--amountOfItems;
		}
		up(CONS_MUTEX, semaphoreId);
	}
	
	if(atoi(argv[1]) == 3)					//producent
	{
		int amountOfItems = atoi(argv[2]);
		
		if(amountOfItems < 0)
		{
			cout<<"Niepoprawny argument\n";
			return -1;
		}
		
		down(PROD_MUTEX, semaphoreId);
		buf_ptr = (int *)shmat(sharedMemoryId, NULL, 0);
		
		while( amountOfItems > 0 )
		{	
			down(EMPTY, semaphoreId);
			down(MUTEX, semaphoreId);
			cout<<"Z adresu bufora nr: "<<write_ptr<<endl;
			buf_ptr[write_ptr] = amountOfItems*2;

			write_ptr = (write_ptr + 1)%BUFOR_SIZE;

			cout << "Producent: " << getpid() << " wyprodukowal element: " << amountOfItems*2 <<endl;
			sleep (1);
			up(MUTEX, semaphoreId);
			up(FULL, semaphoreId);
			--amountOfItems;
		}
		up(PROD_MUTEX, semaphoreId);
	}

	if(atoi(argv[1]) == 1 && atoi(argv[2]) == 2)
	{
		
	
		semctl(semaphoreId, 0, IPC_RMID);
		if(shmctl(sharedMemoryId, IPC_RMID, NULL) != -1)
			cout<<"Zwolnienie pamieci wspoldzielonej i semaforow zakonczone powodzeniem\n";
		else
			cout<<"Nie udalo sie zwolnic pamieci.\n";
	}
	return 0;
}


		
		
