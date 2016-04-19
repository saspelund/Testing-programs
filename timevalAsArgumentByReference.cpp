#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>

int print_time(timeval *currentTime)
{
	printf("currentTime1: %ld & %ld\n",currentTime->tv_sec,currentTime->tv_usec);
	
	usleep(1000000);
	gettimeofday(currentTime, NULL);
	
	return 0;
}

int main()
{
	struct timeval currentTime;

	gettimeofday(&currentTime, NULL);
	
	printf("currentTime0: %ld & %ld\n",currentTime.tv_sec,currentTime.tv_usec);

	usleep(1000000);

	print_time(&currentTime);
	
	printf("currentTime2: %ld & %ld\n",currentTime.tv_sec,currentTime.tv_usec);
	
	return 0;
}
