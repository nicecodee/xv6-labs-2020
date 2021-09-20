#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{
	int seconds;
	
	if(argc !=2 || strcmp(argv[0],"sleep") != 0){  //易错点：字符串的比较要用strcmp,不能用 argv[0] != "sleep" 作比较("sleep"和argv[0]指向的地址是不一样的) 
		fprintf(2, "Usage: sleep <number of seconds>\n");
		exit(1);
	}
	
	seconds = atoi(argv[1]);
	if( seconds > 0 ){
		fprintf(1, "Sleeping...\n");
		sleep(seconds);
	}

	exit(0);
}

