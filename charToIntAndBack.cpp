#include <stdio.h>
#include <string.h>


void endian_swap_32(unsigned int & x)
{
    x = (x>>24) |
        ((x<<8) & 0x00FF0000) |
        ((x>>8) & 0x0000FF00) |
         (x<<24);
}



int main()
{
/*	unsigned char asChar = 0xff;
	
	printf("asChar = %x\n",asChar);
	
	unsigned int asInt = (unsigned int) asChar;
	
	printf("asInt = %d\n",asInt);
*/	
	
	unsigned char floatAsCharBE[] = {0x49,0xeb,0x65,0xb0}; //1928374
	
	endian_swap_32( (unsigned int&) floatAsCharBE[0] );
	
	for (int i=0;i<4;i++)
		printf("%x\n",floatAsCharBE[i]);
	
	float floatAsFloat;
	//memcpy = (float) floatAsCharBE[0];
	
	memcpy( &floatAsFloat,  &floatAsCharBE[0], sizeof( float ) );
	
	printf("\n%f\n",floatAsFloat);
	
	
	return 0;
}
