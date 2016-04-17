//This program should generate random XY coordinates for testing purposes
// Compile with     g++ RandomXYGenerator.cpp -o RandomXYGenerator
//---------------------------------------------------------------

#include <stdlib.h>
#include <time.h>
#include <iostream> // not needed if not using cout


/* change these values to match your plot */
#define XMIN 0
#define XMAX 5
#define YMIN 0
#define YMAX 5

int main ()
{
	double X, Y;
	
	srand (time(NULL));
	
	X = ( rand() % ( (XMAX - XMIN)*100 ) )/100.0 + XMIN;
	Y = ( rand() % ( (YMAX - YMIN)*100 ) )/100.0 + YMIN;
	
	std::cout << X << " " << Y << std::endl; // Replace this line with the function call to plot
	
	return 0;
}



