#include <stdio.h>
#include <stdlib.h>	//to use system()
	
int main()
{
	char* command = "date";
	printf("date/time is ...");
	system(command);
	
	return 0;
}