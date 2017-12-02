/* THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Hang Jiang */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <ctype.h>
#include <errno.h>
#define PIPE_BUF        4096

int main(int argc, char *argv[]) {
	int pfd[2], sfd[2];
	FILE *stream, *stream2, *stream3;
	pid_t status1, status2;

	/* Create the pipe. */
	if (pipe (pfd))
    {
      fprintf (stderr, "Proprocess-Sorting Pipe failed.\n");
      exit(-1);
    }

    if (pipe (sfd))
    {
      fprintf (stderr, "Suppressor-Sorting Pipe failed.\n");
      exit(-1);
    }

    // suppressor child process
    pid_t sup_cid;
    sup_cid = fork();
    if (sup_cid == 0) {
    	// printf("-----Suppressor Process-----\n");
    	// close irrelevant pipe fds
    	close(pfd[0]);
    	close(pfd[1]);

    	// handle sfd pipe
    	close(sfd[1]);
    	dup2(sfd[0],0);
    	close(sfd[0]);

    	char currWord[37];
    	char nextWord[37];
    	int wcount = 1;

    	fgets(currWord, 37, stdin);
    	// printf("%-5d%s", wcount, currWord);
    	while (fgets(nextWord, 37, stdin) != NULL) {
    		if (strcmp(currWord, nextWord) == 0) {
    			wcount++;
    			continue;
    		}
    		printf("%-5d%s", wcount, currWord);
    		wcount = 1;
    		strcpy(currWord, nextWord);
    		memset(nextWord, '\0', 37);
    	}
    	printf("%-5d%s", wcount, currWord);
    	// printf("Message from the suppressor process.\n");
    	exit(0);
    }

    // sort child process
    pid_t sort_cid;
    sort_cid = fork();
    
    if (sort_cid == 0) {
    	// printf("-----Sorting Process-----\n");
    	// handle pfd pipe
    	close(pfd[1]);
    	dup2(pfd[0], 0);
    	close(pfd[0]);

    	// handle sfd pipe
    	close(sfd[0]);
    	dup2(sfd[1], 1);
    	close(sfd[1]);

    	execl("/usr/bin/sort", "sort", (char *) NULL);
    	// printf("Message from sorting process!\n");
    	exit(1);
    }

    // parent process
    // printf("-----Parent------\n");

    char c;
    int count = 0;
    char buff[37]; // 35 + \n + NULL
    close(pfd[0]);
    stream = fdopen(pfd[1],"w");
    while ((c = fgetc(stdin)) != EOF) {
    	// pick letters only
    	if (isalpha(c)) {
    		if (count < 35) {
    			buff[count++] = tolower(c);
    		}
    	} else {
    		if (count >= 5) {
    			// store in buff 
    			buff[count++] = '\n';
    			buff[count++] = '\0';
    			fputs(buff, stream);
    		} 
    		memset(&buff, '\0', 37);
    		count = 0;
    	}
    }

    fclose(stream);
    close(pfd[1]);
    close(sfd[0]);
    close(sfd[1]);
    wait(&status1);
    wait(&status2);
    return 0;

}
















