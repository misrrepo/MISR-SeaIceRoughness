#include <stdio.h>

int main(int argc, char* argv[])
{
	printf("number of args: %d \n" , argc);

	printf("we receive these names from cmd-Line \n");
	printf("name 0: %s \n" , argv[0]);
	printf("name 1: %s \n" , argv[1]);
	printf("name 2: %s \n" , argv[2]);
	printf("name 3: %s \n" , argv[3]);
	return 0;
}