#include <iostream>
#include <stdio.h>


using namespace std;

int main()
{
	double value;
	cout << "Give me a number with a decimal point: ";
	cin >> value;
	
	printf( "The number you gave me was: %f\n", value);
	printf( "The number times 1,000,000: %f\n", value*1000000);
	
	return 0;
}
