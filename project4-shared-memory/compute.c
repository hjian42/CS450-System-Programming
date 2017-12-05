/*THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR  
OR CODE WRITTEN BY OTHER STUDENTS - Hang Jiang*/

#include "share.h"
sharedMemory_struct *shmem;
int shmID;
int msgID;
int idxOfProcess;

int isPerfect(int curr) {
	if (curr < 2) {
		return 0;
	} 
	int sum = 1;
	int i;
	for (i=2;i<curr/2+1;i++)
		if (!(curr%i)) sum+=i; 
	if (sum == curr) return 1;
	return 0;
}

// test whether the process is emptied
void testEmpty(){
	// test whether one process has been cleaned totally
	printf("pid: %d\n", shmem->processes[idxOfProcess].pid);
	printf("untested: %d\n", shmem->processes[idxOfProcess].numberOfNotTested);
	printf("tested: %d\n", shmem->processes[idxOfProcess].numberOfTested);
	printf("perfect: %d\n", shmem->processes[idxOfProcess].numberOfPerfect);
}

void terminate(int sig){
	if (idxOfProcess < 0 | idxOfProcess >= 20)
		exit(1);
	// erase record on shared memory
	memset(&shmem->processes[idxOfProcess], 0, sizeof(process_struct));
	
	// test empty
	// testEmpty();

	exit(0);
}


int main(int argc, char* argv[]) {

	// read the input start number
	int start;
	if (argc != 2) {
		start = 2;
	} else {
		start = atoi(argv[1]);
	}

	// signal handler
	struct sigaction act; //sigaction object to handle signals
	act.sa_handler = terminate;
	if (sigaction(SIGINT, &act, NULL) == -1) {
        perror("sigaction error");
        exit(1);
    }
    if (sigaction(SIGHUP, &act, NULL) == -1) {
        perror("sigaction error");
        exit(1);
    }
    if (sigaction(SIGQUIT, &act, NULL) == -1) {
        perror("sigaction error");
        exit(1);
    } 

	// connect to shared memory
	shmID = shmget(MKEY, sizeof(sharedMemory_struct), 0666);
	shmem = shmat(shmID, NULL, 0);

	// connect to message queue
	msgID = msgget(QKEY, 0666);
    message_struct *msg_buffer;
    msg_buffer = malloc(sizeof(message_struct));

    // send the reigster message at type 1
    msg_buffer->msg_type = PERFECT_MSG;
    msg_buffer->content = getpid();
    int nread = msgsnd(msgID,msg_buffer,sizeof(msg_buffer->content),0);

    // get idx of process when type 3
    msgrcv(msgID, msg_buffer, sizeof(msg_buffer->content), PROCESS_INDEX_MSG, 0);
    idxOfProcess = msg_buffer->content;

	// find perfect numbers in a loop 
	int curr = start;
	while (1) {
		// exlcude 0 and 1
		int segIdx = (curr-2)/32;
		int bitIdx = (curr-2)%32;

		// if 0: untested, else (nonzero): tested
		if (!(shmem->bitmap[segIdx]&(1<<bitIdx))) {

			if(isPerfect(curr)) {

				// update perfect number to manage.c
				msg_buffer->msg_type = REGISTER_MSG;
				msg_buffer->content = curr;
				msgsnd(msgID, msg_buffer, sizeof(msg_buffer->content),0);
				printf("Updated perfect number: %d\n", curr);

				// update to shared memory
				shmem->processes[idxOfProcess].numberOfPerfect++;
			}
			shmem->processes[idxOfProcess].numberOfTested++; 
			shmem->bitmap[segIdx] |= (1<<bitIdx); // mark as tested
		} else {
			shmem->processes[idxOfProcess].numberOfNotTested++; // number of skipped bits
		}
		curr++;
	}

}













