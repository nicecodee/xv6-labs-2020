#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

/*
prime = get a number from left neighbor
print prime
loop:
    n = get a number from left neighbor
    if (prime does not divide n)
        send n to right neighbor
*/

void child_process(int p[2]) {
    int len;
    int prime;
    int num;
    int pp[2];
    // no need to write to left neighbor(its parent)
    close(p[1]);	
    //read returns zero when the write-side of a pipe is closed
    len = read(p[0], &prime, sizeof(int));
    if (len == 0) {
        close(p[0]);
        exit(0);
    }
    //Data written to p[1] is read from p[0] on a FIFO basis, so the 1st number in p[0] must be a prime
    printf("prime %d\n", prime); 
    pipe(pp);  
    // create right neighbor
    if (fork() == 0) {    //at this moment, the right neighbor(the newly-forked child) has p[0], pp[0] and pp[1] 
        // right neighbor don't need its parent's p[0]
        close(p[0]);
        child_process(pp);
    } else {
        close(pp[0]);   // no need to read from right neighbor
        while (1) {
            // read from left neighbor(it's parent)
            len = read(p[0], &num, sizeof(int));
            if (len == 0) {
                break;
            }
            // filter the number
            if (num % prime != 0) {
                // write to right neighbor
                write(pp[1], &num, sizeof(int));
            }
        }
        // release all of its fds
        close(p[0]);   
        close(pp[1]);
        //wait for its child exit 
        wait(0);
    }
    exit(0);
}
int main(int argc, char *argv[])
{
	int i;
	int p[2];
	if(argc > 1){
		fprintf(2, "Usage: primes\n");
		exit(1);
	}
	
	pipe(p);
	// first round
	// generate 2~35 to child process
	if (fork() == 0) {
	  child_process(p);
	} else {
	  // no need to read from left neighbor
	  close(p[0]);
	  for (i = 2; i <= 35; i++) {
	      write(p[1], &i, sizeof(int));
	  }
	  close(p[1]);
	  wait(0);
	}
	exit(0);
}
