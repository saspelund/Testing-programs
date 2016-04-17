#include <stdio.h>

unsigned char calcCheckSum( unsigned char * startAddress, int numberOfElements)
{
	unsigned int checkSum = 0;
	for (int i=0; i<numberOfElements; i++)
	{
		checkSum += (unsigned int)startAddress[i]; //assuming it is being passed less than 2^24-1 elements
	}
	
	return (unsigned char)(checkSum & 0xff);
}




int main( int argc , char ** argv)
{
	unsigned char msg[6];
	unsigned int device = 2;
	
	msg[0] = 0xf7;
	msg[1] = (unsigned char)(device); // I've read this is bad form but I will fix it later. I don't even know if it'll work correctly
	msg[2] = 0x50;
	msg[3] = 0xa0;
	msg[4] = 0x10;
	
	
	//I really need to wrie a checksum function
	//msg[11] = checksum(msg[1] through msg[10]);
	//how about this:
	msg[5] = calcCheckSum ( & msg[1], 4);
	
	printf("checksum = %x\n", msg[5]);
	
	return 0;
		
	
	
}
