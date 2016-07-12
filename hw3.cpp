#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>  
#include <string> 

pthread_cond_t cond     = PTHREAD_COND_INITIALIZER; 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  

char seat[10][40];
int n;

struct threads{
	pthread_t t;
	char seller;
	int number;
	int *queue;
	int *customer;
	int timer;
	int seated;
	int turnedAway;
}threads;

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

void initQueues(int *queue)
{
	for(int i = 0; i < n; i++)
	{
		queue[i] = 0;
	}
}

void enqueue(int *queue, int value)
{
	for(int i = 0; i < n; i++)
	{
		if(queue[i] == 0)
		{
			queue[i] = value;
			break;
		}
	}
}

void dequeue(int *queue)
{
	for(int i = 0; i < n-1; i++)
	{
		queue[i] = queue[i+1];
	}
	queue[n-1] = 0;
}

bool queueIsEmpty(int *queue)
{	
	for(int i = 0; i < n; i++)
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
void createCustomerArray(int *customers)
{
	int i, j;
	for(i = 0; i < n; i++)
	{
		int r = rand() % (59 + 1 - 1) + 1;
		customers[i] = r;
	}
}

void sortCustomerArray(int *customers)
{
	bool swapped = false;
	while(!swapped)
	{
		swapped = true;
		for(int j = 0; j < n-1; j++)
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

void printCustomers(int *queue)
{
	for(int i = 0; i < n; i++)
	{
		printf("%i ",queue[i]);
	}
	printf("\n");
}

void minutesPassed(int &timer, char seller)
{
	int minutes;
	if(seller =='H')
	{
		minutes = rand() % (1 + 1 - 1) + 1;
		timer += minutes;
		sleep(minutes/100000);
	}
	else if(seller == 'M')
	{
		minutes = rand() % (4 + 1 - 2) + 2;
		timer += minutes;
		sleep(minutes/100000);
	}
	else
	{
		minutes = rand() % (7 + 1 - 4) + 4;
		timer += minutes;
		sleep(minutes/100000);
		
	}
}

void customersTurnedAway(struct threads *t)
{
	for(int i = 0; i < n; i++)
	{
		if(t->customer[0] > 0 )
		{
			t->turnedAway++;
		}
		if(t->queue[0] > 0 )
		{
			t->turnedAway++;
		}
		dequeue(t->customer);
		dequeue(t->queue);
	}
}

void wakeup_all_seller_threads() 
   { 
         pthread_mutex_lock(&mutex); 
         pthread_cond_broadcast(&cond);     
         pthread_mutex_unlock(&mutex); 
   }

// seller thread to serve one time slice (1 minute) 
void *sell(void *threads)
 {
 	struct threads *t = (struct threads *) threads;

 	while(t->timer < 60)
 	{
 		t->timer++;
 		bool all = false;
 		while(!all)
 		{
 			all = true;
			if(t->timer >= t->customer[0] && t->customer[0] != 0)
			{
				all = false;
				printf("Time %i - Customer %i arrived in %c%i's queue\n\n", t->timer, t->customer[0],t->seller,t->number);
				enqueue(t->queue, t->customer[0]);
				dequeue(t->customer);
 			}
 		}

 		while(!queueIsEmpty(t->queue) && t->timer < 60)
 		{
 			pthread_mutex_lock(&mutex);
			pthread_cond_wait(&cond, &mutex);

			minutesPassed(t->timer, t->seller);

    		addSeat(t->seller, t->number, t->queue[0]);
    		printf("Time %2i - Customer %i bought a ticket at %c%i%i%i\n          Customer %i leaves\n\n",t->timer, t->queue[0], t->seller, t->number,(t->queue[0]/10), (t->queue[0]%10),t->queue[0]);
    
    		dequeue(t->queue);
    		pthread_cond_signal(&cond);
  			pthread_mutex_unlock(&mutex);
  			t->seated++;
   		}
	}

	customersTurnedAway(t);
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
    srand(time(NULL));

 	printf("N = ");
 	scanf("%i", &n);
   
     // Create necessary data structures for the simulator. 
	struct threads t[10];
	for(int i = 0; i < 10; i++)
	{
		t[i].customer = (int*) malloc(n * sizeof(int));
		t[i].queue = (int*) malloc(n * sizeof(int));;
	}
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
    t[i].seated = 0;
    t[i].turnedAway = 0;
    initQueues(t[i].queue);
    createCustomerArray(t[i].customer);
    sortCustomerArray(t[i].customer); 

    pthread_create(&tids[i], NULL, sell, &t[i]);
    //wakeup_all_seller_threads();

    seller_type = 'M';
    number = 0;
	  
	for (i = 1; i < 4; i++)
	{
	 	number++;
	 	t[i].t = tids[i];
     	t[i].seller = seller_type;
     	t[i].number = number;
     	t[i].timer = 0;
     	t[i].seated = 0;
     	t[i].turnedAway = 0;
	 	initQueues(t[i].queue);
	 	createCustomerArray(t[i].customer);
	 	sortCustomerArray(t[i].customer); 

	   	pthread_create(&tids[i], NULL, sell, &t[i]);
	   	//wakeup_all_seller_threads();
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
     	t[i].seated = 0;
     	t[i].turnedAway = 0;
	 	initQueues(t[i].queue);
	 	createCustomerArray(t[i].customer);
     	sortCustomerArray(t[i].customer);    

	  	pthread_create(&tids[i], NULL, sell, &t[i]); 
	  	//wakeup_all_seller_threads();
	}

	 // wakeup all seller threads    
	wakeup_all_seller_threads();
	     
	 // wait for all seller threads to exit
	 for (i = 0 ; i < 10 ; i++)
	    pthread_join(tids[i], NULL);


	 // Printout simulation results
	printSeats();

	for(int i = 0; i < 10; i++)
	{
		printf("Number of tickets %c%i sold: %i\n", t[i].seller, t[i].number, t[i].seated);
		printf("Number of customers turned away: %i\n\n", t[i].turnedAway);
	}
	 	
	 	exit(0); 
}


