/*THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR  
OR CODE WRITTEN BY OTHER STUDENTS - Hang Jiang*/

#include "share.h"

sharedMemory_struct *shmem;
int shmID;
int msgID;

// testing whether the variables in process_struct are all intialized as 0s
void testEmpty(int idxOfProcess){
	// test whether one process has been cleaned totally
	printf("pid: %d\n", shmem->processes[idxOfProcess].pid);
	printf("untested: %d\n", shmem->processes[idxOfProcess].numberOfNotTested);
	printf("tested: %d\n", shmem->processes[idxOfProcess].numberOfTested);
	printf("perfect: %d\n", shmem->processes[idxOfProcess].numberOfPerfect);
}


void initialize_shared_memory() {
	memset(shmem->bitmap, '\0', sizeof(shmem->bitmap));
	memset(shmem->perfectNumsFound, '\0', sizeof(shmem->perfectNumsFound));
	memset(shmem->processes, '\0' , sizeof(shmem->processes));
	shmem->manager_pid = getpid();

	// test 
	// testEmpty(0);
	
} 

void handler_killAll(int sig)
{
    // printf("%s\n", "KILLLLLLLLLLING IS FUN.");

    // kill all compute processes 
    int i;
    for (i=0;i<20;i++)
    	if (shmem->processes[i].pid)
    		kill(shmem->processes[i].pid, SIGINT);

    // sleep 5s
    sleep(5);

    // detach and destroy the shared memory after it is done
	shmdt(shmem);
	shmctl(shmID, IPC_RMID, NULL);
	// kill message queue
	msgctl(msgID, IPC_RMID, NULL);

	exit(0);
}

int main(int argc, char *argv[]){

	// create a shared memory
	shmID = shmget(MKEY, sizeof(sharedMemory_struct), 0666|IPC_EXCL|IPC_CREAT);
	if (shmID == -1) {
		perror("WRONG IN ALLOCATING SHARED MEMORY.");
		exit(1);
	}

	// get the pointer to the shared memory
	shmem = shmat(shmID, NULL, 0);

	// initialize the shared memory
	initialize_shared_memory();

	// handle signals
	struct sigaction act; //sigaction object to handle signals
	act.sa_handler = handler_killAll;
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

    // create to a message queue for receiving from compute.c
    if ((msgID = msgget(QKEY, 0666|IPC_CREAT)) == -1) {
        perror("msgget");
        exit(1);
    }

    // malloc space for message
    message_struct *msg_buffer;
    msg_buffer = malloc(sizeof(message_struct)); // use msg_buffer as pointer

    // alternative way of msg
    // message_struct msg_buffer; // use &msg_buffer later as pointer


    // loop to receive messages from compute.c
    while (1) {
    	// clean the message 
    	memset(msg_buffer, '\0', sizeof(message_struct)); // input of sizeof should be datatype NOT pointer?

    	// receive the message
    	/*sizeof() in msgrcv() is confusing because it means the sizeof content variable in the buffer
    	the first long variable is always there by default*/
    	int nread;
    	// -2 means it receives both type 1, 2 but not 3
    	if ((nread = msgrcv(msgID, msg_buffer, sizeof(msg_buffer->content), -2, 0)) == -1) {
            perror("msgrcv");
            exit(1);
        }

    	printf("Message Received is: type:%-10ldcontent:%-10dsize:%-10d\n", msg_buffer->msg_type, msg_buffer->content, nread);

    	// check the msg type
    	int j;
    	if (msg_buffer->msg_type == PERFECT_MSG) { // register message
    		printf("message is: type %ld\n", msg_buffer->msg_type);
    		for (j=0;j<20;j++) {
    			if(shmem->processes[j].pid == 0) {
    				shmem->processes[j].pid = msg_buffer->content;
    				break;
    			}
    		}
    		// if more than 20 processes: quit
    		// if (j>20) {
    		// 	printf("20 processes already, full!\n");
    		// 	break;
    		// }
    			

    		// update processIndex to compute process
    		msg_buffer->msg_type = PROCESS_INDEX_MSG; // type 3: process index
    		msg_buffer->content = j;
    		msgsnd(msgID, msg_buffer, sizeof(msg_buffer->content), 0);
    	} 
    	else if (msg_buffer->msg_type == REGISTER_MSG) { // content message
    		printf("%ld\n", msg_buffer->msg_type);

    		// remove duplicates


    		// update the perfect number if it is new
    		for (j=0;j<20;j++) {
    			if(shmem->perfectNumsFound[j] == 0) {
    				shmem->perfectNumsFound[j] = msg_buffer->content;
    				break;
    			}
    		}
    		// if (j>20) {
    		// 	printf("20 perfect numbers already, full!\n");
    		// 	break;
    		// }
    	} 
    	
    }



	
    return 0;
}















