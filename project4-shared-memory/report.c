/*THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR  
OR CODE WRITTEN BY OTHER STUDENTS - Hang Jiang*/

// make a make file
// how multiple signals are handled? not simultaneously, right?
// different numbers in exit(num)
// what happen if no exit() in handler? go back to the while loop?
// adjust some functions
// add error handling
// Processes that hit the end should wrap around, but stop at their starting point.???

#include "share.h"
sharedMemory_struct *shmem;
int shmID;
int msgID;
int idxOfProcess;

int main(int argc, char *argv[]) {

	// connect to shared memory
	shmID = shmget(MKEY, sizeof(sharedMemory_struct), 0666);
	if (shmID  == -1){
		perror("Error accessing shared memory.");		
		exit(-1);
	} 
	shmem = shmat(shmID, NULL, 0);


    // print perfect numbers
    int i;
    printf("%s\n", "Perfect Number Found:\t");
    for(i=0; i<20; i++) {
    	if (shmem->perfectNumsFound[i]) {
    		printf("%d ", shmem->perfectNumsFound[i]);
    	}
    }
    printf("\n===================\n");

    // print the table 
    int count = 0;
    printf("\nProcess Table:\npid\ttested\tskipped\tfound\n");
    for (i=0; i<20; i++) {
    	if (shmem->processes[i].pid <= 0) continue;
    	printf("%d\t%d\t%d\t%d\t\n", 
    		shmem->processes[i].pid,
    		shmem->processes[i].numberOfTested,
    		shmem->processes[i].numberOfNotTested,
    		shmem->processes[i].numberOfPerfect);
    	count++;
    }
    printf("\n");

    // report total of the numbers
    int total_tested = 0;
    int total_skipped = 0;
    int total_perfect = 0;
    printf("===================\n");
    printf("\nTotal:\ntested\tskipped\tfound\n");
   	for (i=0; i<20; i++) {
    	if (shmem->processes[i].pid <= 0) continue;
    	total_tested += shmem->processes[i].numberOfTested;
    	total_skipped += shmem->processes[i].numberOfNotTested;
    	total_perfect += shmem->processes[i].numberOfPerfect;
    }
    printf("%d\t%d\t%d\t\n", total_tested, total_skipped, total_perfect);

    // kill processes if -k
    if (argc >= 2){
    	if (strcmp(argv[1],"-k")==0)
    		kill(shmem->manager_pid, SIGINT);
    }

}






















































