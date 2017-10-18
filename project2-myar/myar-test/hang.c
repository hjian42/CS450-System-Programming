// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Hang Jiang

// command: myar option archiveName files
#include <stdio.h>
#include <stdlib.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>

void printInstructions();
void listFiles(char* archiveFile, int isVerbose);


int main(int argc, char *argv[]) {
	if (argc < 3) {
		printInstructions();
		exit(-1);
	}

	printf("This is a test.\n");

	char option = argv[1][0];
	char* archiveFile = argv[2];
	char* files[argc-3];
	if (argc > 3) {
		for (int i=0; i < argc-3; i++) {
			files[i] = argv[i+3];
			printf("%s\n", files[i]);
		}
	}

	switch(option) {
		case 'q':
			break;
		case 'x':
			break;
		case 't':
			listFiles(archiveFile, 0);
			break;
		case 'v':
			listFiles(archiveFile, 1);
			break;
		case 'd':
			break;
		case 'A':
			break;
		default:
			printf("wrong option %c; please look up the following instructions:\n", option);
			printInstructions();
	}
}


void printInstructions(){
	printf("usage:\n");
	printf("q\tquickly append named files to archive\n");
	printf("x\textract named fiels\n");
	printf("t\tprint a concise table of contents of the archive\n");
	printf("v\tprint a verbose table of contents of the archive\n");
	printf("d\tdelete named files from archive\n");
	printf("A\tquickly append all ordinary files in the current directory that have been modified within the last two hours (except the archive itself).\n");

}

void listFiles(char* archiveFile, int isVerbose) {
	int fd = exist(archiveFile);
	if ( fd == -1) {
		return -1;
	}




}


















