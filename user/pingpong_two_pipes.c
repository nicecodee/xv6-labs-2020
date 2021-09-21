#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
    
int
main(void)
{
	char buf[]={'a'};
	// we need 2 pipes, each provides one read end and one write end
	int p1[2];  // p1: parent write, child read 
	int p2[2];  // p2: child write, parent read 
	
	if(pipe(p1) < 0 || pipe(p2)){
		fprintf(2, "pipe error\n");
	}
    
	write(p1[1], buf, 1);  //parent send 1 byte to child via p1[1]
	if(fork() == 0){
		if( read(p1[0], buf, 1) == 1 ){   //get 1 byte sent by parent from p1[0]
			fprintf(1, "%d: received ping\n", getpid());
			write(p2[1], buf, 1);  // send 1 byte via p2[1] to the parent
			exit(0);
		}else{
			fprintf(2, "%d: No ping received\n", getpid());
			exit(1);
		}      
    }else{  // parent read 1 byte from the child
    	wait(0);   //wait until child sent 1 byte to p2[1] and exit normally
		if( read(p2[0], buf, 1) == 1 ){   //get 1 byte sent by child from the pipe
			fprintf(1, "%d: received pong\n", getpid());		
		}else{
			fprintf(2, "%d: No pong received\n", getpid());
			exit(1);
		}  
		// free all the fds of 2 pipes
		close(p1[0]);
		close(p1[1]);
		close(p2[0]);
		close(p2[1]);		
	}
	
	exit(0);
}
