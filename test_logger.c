#include "logger.h"
#include <unistd.h>
#include<stdio.h>

//Before runnig open in another console listener on 90909
// type: "nc -l 90909"

int main()
{

	int i = 0;
	printf("main() init\n");
	char message[256];
	
	if(!logger_init())
		return 0;
	sleep(5);
	while(1)
	{
		for(int j = 0; j < 50; ++j)
		{
			sprintf(message, "%d Log Message HY ", ++i);
			log_write(message);
			printf("main()message 1\n");
		}
		sleep(1);
			
	}	
	
	return 0;
}