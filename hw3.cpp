#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>  
#include <string> 

pthread_cond_t cond     = PTHREAD_COND_INITIALIZER; 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  

struct threads{
	pthread_t t;
	char seller;
	int number;
	int queue[10];
	int customer[10];
	int timer;
}threads;

char seat[10][40];

void initSeats()
{
	for(int i = 0; i < 10; i++)
	{
		for(int j = 0; j < 40; j++)
		{
			seat[i][j] = '-';
		}
	}
}

void initQueues(int queue[10])
{
	for(int i = 0; i < 10; i++)
	{
		queue[i] = 0;
	}
}

 bool seatsAreFull()
 {
 	for(int i = 0; i < 10; i++)
 	{
 		for(int j = 0; j < 40; j+=4)
 		{
 			if(seat[i][j] == '-')
 			{
 				return false;
 			}
 		}
 	}
 	return true;
 }

void printSeats()
{ 	
	int every4 = 0;
	for(int i = 0; i < 10; i++)
 	{
 		for(int j = 0; j < 40; j++)
		{
			if(every4 == 4)
			{
				printf("\t");
				every4 = 0;
			}
			printf("%c", seat[i][j]);
			every4++;
		}
		printf("\n");
		every4 = 0;
 	}
 	printf("\n");
 }


void wakeup_all_seller_threads() 
   { 
         pthread_mutex_lock(&mutex); 
         pthread_cond_broadcast(&cond);     
         pthread_mutex_unlock(&mutex); 
   }

void addSeat(char seller, int number, int customer)
{
	bool valid = false;
	if(seller == 'H')
	{
		for(int i = 0; i < 10; i++)
 		{
 			for(int j = 0; j < 40; j+= 4)
 			{
 				if(seat[i][j] == '-')
 				{
 					seat[i][j] = seller;
 					seat[i][j+1] = '0' + number;
 					seat[i][j+2] = '0' + (customer/10);
 					seat[i][j+3] = '0' + (customer%10);
 					valid = true;
 					break;
 				}
 			}
 			if(valid)
 			{
 				break;
 			}
 		}
 	}
 	else if(seller == 'M')
 	{
 		int modifier = 0;
 		for(int i = 4; i < 10; i+= modifier)
 		{
 			for(int j = 0; j < 40; j+= 4)
 			{
 				if(seat[i][j] == '-')
 				{
 					seat[i][j] = seller;
 					seat[i][j+1] = '0' + number;
 					seat[i][j+2] = '0' + (customer/10);
 					seat[i][j+3] = '0' + (customer%10);
 					valid = true;
 					break;
 				}
 			}
 			if(valid)
 			{
 				break;
 			}
 			if(modifier == 0)
 			{
 				modifier++;
 			}
 			else if((modifier%2) == 0 || (modifier%2) == -1)
 			{
 				modifier = 0 - modifier;
 				modifier++;
 			}
 			else
 			{
 				modifier++;
 				modifier = 0 - modifier;
 			}
 		}
 	}
 	else
 	{
 		for(int i = 10; i > 0; i--)
 		{
 			for(int j = 0; j < 40; j+= 4)
 			{
 				if(seat[i][j] == '-')
 				{
 					seat[i][j] = seller;
 					seat[i][j+1] = '0' + number;
 					seat[i][j+2] = '0' + (customer/10);
 					seat[i][j+3] = '0' + (customer%10);
 					valid = true;
 					break;
 				}
 			}
 			if(valid)
 			{
 				break;
 			}
 		}
 	}
}

void enqueue(int queue[10], int value)
{
	for(int i = 0; i < 10; i++)
	{
		if(queue[i] == 0)
		{
			queue[i] = value;
			break;
		}
	}
}

void dequeue(int queue[10])
{
	for(int i = 0; i < 9; i++)
	{
		queue[i] = queue[i+1];
	}
	queue[9] = 0;
}

bool queueIsEmpty(int queue[10])
{	
	for(int i = 0; i < 10; i++)
	{
		if(queue[i] > 0)
		{
			return  false;
		}
	}
	return true;
}

//A random time will be given to each customer 
//in order for them to get in the seller queue
void createCustomerArray(int customers[10])
{
	int i, j;
	for(i = 0; i < 10; i++)
	{
		customers[i]= rand() % (59 + 1 - 1) + 1;
	}
}

void sortCustomerArray(int customers[10])
{
	bool swapped = false;
	while(!swapped)
	{
		swapped = true;
		for(int j = 0; j < 9; j++)
		{
			if(customers[j] > customers[j+1])
			{
				int temp = customers[j];
				customers[j] = customers[j+1];
				customers[j+1] = temp;
				swapped = false;
			}
		}
	}
}

void printCustomers(int queue[10])
{
	for(int i = 0; i < 10; i++)
	{
		printf("%i ",queue[i]);
	}
	printf("\n");
}

void minutesPassed(	int &minutes, char seller)
{
	if(seller =='H')
	{
		minutes = rand() % (1 + 1 - 1) + 1;
		sleep(minutes/100000);
	}
	else if(seller == 'M')
	{
		minutes = rand() % (4 + 1 - 2) + 2;
		sleep(minutes/100000);
	}
	else
	{
		minutes = rand() % (7 + 1 - 4) + 4;
		sleep(minutes/100000);
		
	}
	pthread_cond_signal(&cond);
}

// seller thread to serve one time slice (1 minute) 
void *sell(void *threads)
 {
 	struct threads *t = (struct threads *) threads;

 	while(!seatsAreFull() && t->timer < 60)
 	{
 		bool all = false;
 		while(!all)
 		{
 			all = true;
			if(t->timer == t->customer[0])
			{
				all = false;
				printf("Time %i - Customer %i arrived in %c%i's queue\n", t->timer, t->customer[0],t->seller,t->number);
				enqueue(t->queue, t->customer[0]);
				dequeue(t->customer);
 			}
 		}

 		while(!queueIsEmpty(t->queue))
 		{
 			int passed = 0;
 			pthread_mutex_lock(&mutex);
			pthread_cond_wait(&cond, &mutex);
    		addSeat(t->seller, t->number, t->queue[0]);
    		printf("Time %i - Customer %i bought a ticket at %c%i%i%i\n",t->timer, t->queue[0], t->seller, t->number,(t->queue[0]/10), (t->queue[0]%10));

    		minutesPassed(passed, t->seller);
    		
    		printf("Time %i - Customer %i leaves\n\n", (t->timer + passed), t->queue[0]);
    		dequeue(t->queue);

  			pthread_mutex_unlock(&mutex);
   		}	
   		t->timer++;
	}
    // Serve any buyer available in this seller queue that is ready             
    // now to buy ticket till done with all relevant buyers in their queue

    return NULL;  // thread exits 
 }

int main()
 {
 	initSeats();

     int i, number;     
   	 pthread_t   tids[10];     
     char  seller_type;   
     

     // Create necessary data structures for the simulator. 
	struct threads t[10];
     // Create buyers list for each seller ticket queue based on the     
     // N value within an hour and have them in the seller queue. 

     // Create 10 threads representing the 10 sellers.
     seller_type = 'H'; 
     i = 0;
     number = 1;

     t[i].t = tids[i];
     t[i].seller = seller_type;
     t[i].number = number;
     t[i].timer = 0;
     initQueues(t[i].queue);
     createCustomerArray(t[i].customer);
     sortCustomerArray(t[i].customer); 

     pthread_create(&tids[i], NULL, sell, &t[i]);

     seller_type = 'M';
     number = 0;
	  
	 for (i = 1; i < 4; i++)
	 {
	 	number++;
	 	t[i].t = tids[i];
     	t[i].seller = seller_type;
     	t[i].number = number;
     	t[i].timer = 0;
	 	initQueues(t[i].queue);
	 	createCustomerArray(t[i].customer);
	 	sortCustomerArray(t[i].customer); 

	   	pthread_create(&tids[i], NULL, sell, &t[i]);
	  }

	 number = 0;
	 seller_type = 'L';

	 for (i =4; i < 10; i++) 
	 {
	 	number++;
	 	t[i].t = tids[i];
     	t[i].seller = seller_type;
     	t[i].number = number;
     	t[i].timer = 0;  
	 	initQueues(t[i].queue);
	 	createCustomerArray(t[i].customer);
     	sortCustomerArray(t[i].customer);     

	  	pthread_create(&tids[i], NULL, sell, &t[i]); 
	  }

	   	pthread_create(&tids[i], NULL, sell, &t[i]);

	 // wakeup all seller threads    
	 wakeup_all_seller_threads();
	     
	 // wait for all seller threads to exit
	 for (i = 0 ; i < 10 ; i++)
	      pthread_join(tids[i], NULL);


	 // Printout simulation results
	  printSeats();
	 	
	 	exit(0); 
}


