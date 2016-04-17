#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <string>
#include <unistd.h>


int openFD ()
{
	int fd_;
	std::string device_ = "/home/aspelun1/Documents/Testing_functions/FileDesc_Transfer_through_functions/TheFileInQuestion";
	const char* path = device_.c_str();
	fd_ = ::open(path, O_RDWR | O_CREAT, 0666);
	
	return fd_;
}

int writeToFD ( int fd_)
{
	char msg[] = "This should hopefully be printed to the file.";
	
	
	int res = ::write( fd_, msg, sizeof(msg) ); // Send to device
    if ( res < sizeof(msg) )
        return -1;
	
	return 0; // no error
}

int closeFD (int fd_)
{
	return ::close(fd_);
}

int main (int argc, char ** argv)
{
	int FD_, result;
	
	FD_ = openFD();
	if (FD_ < 0) 
	{
		printf("Could not open file. FD_ = %d\n", FD_);
		return -1;
	}
	
	result = writeToFD(FD_);
	if (result < 0)
	{
		printf("Could not write to file.");
		return -1;
	}
	
	result = closeFD(FD_);
	if (result < 0) 
	{
		printf("Could not close file.");
		return -1;
	}
	
	return 0;
}
