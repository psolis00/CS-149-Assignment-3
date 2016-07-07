#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>  
#include <string> 

pthread_cond_t cond     = PTHREAD_COND_INITIALIZER; 
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  
int timer = 0;

struct threads{
	pthread_t t;
	char seller;
	int number;
	int queue[10];
	int customer[10];
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
		printf("\n");
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
 		int modifier = 1;
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
 			if((i%2) < -1)
 			{
 				modifier--;
 				modifier = 0 - modifier;
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
	bool valid = true;
	for(int i = 0; i < 10; i++)
	{
		if(queue[i] > 0)
		{
			valid = false;
		}
	}
	return valid;
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

// seller thread to serve one time slice (1 minute) 
void *sell(void *threads)
 {
 	struct threads *t = (struct threads *) threads;
 	while(!seatsAreFull() || timer < 60)
 	{
 		if(timer == t->customer[0])
 		{
 			enqueue(t->queue, t->customer[0]);
 		}
 		if(!queueIsEmpty(t->queue))
 		{

 			pthread_mutex_lock(&mutex);

 			printCustomers(t->queue);
 			if(queueIsEmpty(t->queue) )
 			{
 				printf("\ntrue\n");
 			}

			printf("%i\n", timer);
    		addSeat(t->seller, t->number, t->queue[0]);
    		dequeue(t->queue);
   			printSeats();
   
  		  	pthread_mutex_unlock(&mutex);
  		} 
	}
    // Serve any buyer available in this seller queue that is ready             
    // now to buy ticket till done with all relevant buyers in their queue

    return NULL;  // thread exits 
 }

int main()
 {
 	initSeats();

     int i, j, number;     
   	 pthread_t   tids[10];     
     char  seller_type;   
     struct threads t[10];

     // Create necessary data structures for the simulator. 

     // Create buyers list for each seller ticket queue based on the     
     // N value within an hour and have them in the seller queue. 

     // Create 10 threads representing the 10 sellers.
     seller_type = 'H'; 
     i = 0;
     number = 1;

     t[i].t = tids[i];
     t[i].seller = seller_type;
     t[i].number = number;
     initQueues(t[i].queue);
     createCustomerArray(t[i].customer);
     sortCustomerArray(t[i].queue); 
     pthread_create(&tids[i], NULL, sell, &t[i]);

     seller_type = 'M';
     number = 0;
	  
	 for (i = 1; i < 4; i++)
	 {
	 	initQueues(t[i].queue);
	 	createCustomerArray(t[i].queue);
	 	sortCustomerArray(t[i].queue); 
	 	number++;
	 	t[i].t = tids[i];
     	t[i].seller = seller_type;
     	t[i].number = number;
	   	pthread_create(&tids[i], NULL, sell, &t[i]);
	  }

	 number = 0;
	 seller_type = 'L';

	 for (i = 4; i < 10; i++) 
	 {	number++;
	 	initQueues(t[i].queue);
	 	t[i].t = tids[i];
     	t[i].seller = seller_type;
     	t[i].number = number;
     	createCustomerArray(t[i].queue);
     	sortCustomerArray(t[i].queue);        
	  	pthread_create(&tids[i], NULL, sell, &t[i]); 
	  }

	 // wakeup all seller threads    
	 wakeup_all_seller_threads();
	     
	 // wait for all seller threads to exit
	 for (i = 0 ; i < 10 ; i++)
	      pthread_join(tids[i], NULL);


	 // Printout simulation results
     


	 exit(0); 
}

