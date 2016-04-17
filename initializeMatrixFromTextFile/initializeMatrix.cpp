#include <fstream>
#include <stdio.h>




int main(int argc, char ** argv)
{
	
	
	
	double multiplierMatrix[2][8];
	
	std::ifstream infile(argv[1]);
	
	
	double value;
	for (int row = 0; row <2; row ++)
	{
		for (int col = 0; col <8; col ++)
		{	
			infile >> multiplierMatrix[row][col];
		}
	}
	
	
	printf("row,col\n");
	for (int row = 0; row <2; row ++)
	{
		for (int col = 0; col <8; col ++)
		{	
			printf("%3d,%3d: %4f\n",row,col,multiplierMatrix[row][col]);
		}
	}
	
	
	return 0;
}
