#include <signal.h>
#include <stdio.h>

bool keepGoing = true; //the loops will check this value
void signal_handler(int signum)
{
	keepGoing = false;
	printf("Caught signal %d\n",signum);
}


int main(int argc, char ** argv){
	signal(SIGINT, signal_handler);
	
	long unsigned int counter = 0;
	
	while (keepGoing)
	{
		counter++;
	}
	
	printf("counter = %ld\n",counter);
	
}
