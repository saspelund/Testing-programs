#include <fstream>
#include <stdio.h>


//g++ initializeMatrix2.cpp -o initializeMatrix2

int main(int argc, char ** argv)
{
	//Read in Matrix from a file or I guess we can just hard code it for now
	double multMatrix[8][8];
	double variance[8];
	
	std::ifstream infile("/home/aspelun1/Documents/IMUcode/OnlyNeededFiles/Sanders_matrix_3D.txt");
	
	//double value;
	for (int col = 0; col <8; col ++)
	{
		for (int row = 0; row <8; row ++)
		{	
			infile >> multMatrix[row][col];
		}
		//printf("\n");
	}
	for (int i = 0; i <8; i++)
	{	
		infile >> variance[i];
		//variance[i] = sqrt(variance[i]);
	}
	
	
	printf("row,col\n");
	for (int row = 0; row <8; row ++)
	{
		for (int col = 0; col <8; col ++)
		{	
			printf("%d,%d: %4f\t",row,col,multMatrix[row][col]);
		}
		printf("\n");
	}
	
	for (int i = 0; i <8; i++)
	{	
		printf("%4f\n",variance[i]);
	}
	
	return 0;
}
