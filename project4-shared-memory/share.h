/*THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR  
OR CODE WRITTEN BY OTHER STUDENTS - Hang Jiang*/

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include<sys/types.h>
#include <sys/shm.h>
#include <string.h>
#include <signal.h>
#include <sys/msg.h>

#define MKEY 77700
#define QKEY 88800
#define PERFECT_MSG 1
#define REGISTER_MSG 2
#define PROCESS_INDEX_MSG 3

// struct of each process
typedef struct process_struct{
	pid_t pid;
	int numberOfPerfect;
	int numberOfTested;
	int numberOfNotTested;
} process_struct;

// struct of shared memory space
typedef struct sharedMemory_struct{

	// 0 means untested; 1 means tested already in bitmap
	// int bitmap[1050000]; // final: 2^25 is 33,554,432/32 = 1,048,576
	int bitmap[100000]; // test 
	int perfectNumsFound[20];

	process_struct processes[20];

	int manager_pid; 

} sharedMemory_struct;

// struct of messages

typedef struct message_struct
{
	long msg_type;
	int content;
} message_struct;
















