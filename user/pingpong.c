#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
    

int
main(void)
{
	char buf[1]={'a'};
	int p[2];  
	
	if(pipe(p) < 0){  //as a pipe, p has two ends： p[0] is the read end, p[1] is the write end 
		printf("pipe error\n");
	}
      
	write(p[1], buf, 1);  //parent send 1 byte to child via the write end of the pipe
	if(fork() == 0){ // child 
		if( read(p[0], buf, 1) == 1 ){   //get 1 byte sent by parent from the read end of the pipe
			printf("%d: received ping\n", getpid());
			write(p[1], buf, 1);  // send 1 byte to its parent
			exit(0);
		}else{
			printf("%d: No ping received\n", getpid());
			exit(1);
		}      
    }else{  // parent read the byte from its child
    	wait(0);   //wait until child sent 1 byte and exit normally
		if( read(p[0], buf, 1) == 1 ){   //get 1 byte sent by child 
			printf("%d: received pong\n", getpid());		
		}else{
			printf("%d: No pong received\n", getpid());
			exit(1);
		}  
		// free all the fds of the pipe
		close(p[0]);
		close(p[1]);
	}
	
	exit(0);
    
}

