// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Hang Jiang

// commands I implemented: t q A x v d

#include <stdio.h>
#include <stdlib.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include <errno.h>
#include <ar.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <utime.h>
#include <libgen.h>

void printUsage();

typedef struct ar_hdr Header;

int readNext(int fd, Header* arHeader) {
	int nread = read(fd, arHeader, sizeof(Header));
	if (nread != sizeof(Header)) {
		return -1;
	}
	return nread;
}

int handleString(char* buff, char* src, int size) {
	memset(buff, '\0', size);
	sprintf(buff, "%.*s", size-1, src);
	int i = 0;
	while ( i < size) {
		if (buff[i] == ' ') {
			break;
		} else if (buff[i] == '\0') {
			break;
		} else if (buff[i] == '/') {
			break;
		}
		i++;
	}
	return i;
}

void extract(int fd, char** files, int numOfFiles) {
	Header *arHeader = malloc(sizeof(Header));
	int i;
	char fbuff[sizeof(arHeader->ar_name)];
	int findflag;
	int fileSize;

	// handle extract for multiple files 
	for (i=0; i< numOfFiles; i++) {
		findflag=0;
		// scan archive one pass to find each targeted file only
		while (readNext(fd, arHeader) != -1) {
			// if filename match: extract
			handleString(fbuff, arHeader->ar_name, sizeof(arHeader->ar_name));
			fileSize = (int) atoi(arHeader->ar_size);
			// printf("%d\n", (int)fileSize);
			// printf("%s\n", arHeader->ar_size);
			if (strncmp(fbuff, files[i], strlen(files[i])) == 0) {
				findflag = 1;
				break;
			} else {
				if (fileSize%2==1) fileSize++;
				lseek(fd, fileSize, SEEK_CUR);
			}
		}

		// if not found: continue with
		if (findflag == 0) {
			printf("File %s is not found in archive\n",files[i]);
			continue;
		}

		// if found: extract the file
		char* ptr;
		mode_t mode = (mode_t) strtol(arHeader->ar_mode, &ptr, 8);
		int ffd = creat(files[i], mode);
		int n_read;

		struct stat *f_stat = malloc(sizeof(struct stat));
		fstat(fd, f_stat);
		int buf_size = (int)f_stat->st_blksize;

		// write the content 
		while (fileSize>0) {
			if (fileSize < buf_size) {
				buf_size = fileSize;
			} 
			char* buf[buf_size];
			if ((n_read=read(fd, buf, buf_size)) > 0) {
				if (write(ffd, buf, n_read) != n_read) {
					perror("write error");
					exit(-1);
				}
			}
			fileSize = fileSize - buf_size;
		}

		// restore timestamp 
		struct utimbuf* tbuff=(struct utimbuf*) malloc(sizeof(struct utimbuf));	
		time_t timestamp = (time_t) atoll (arHeader->ar_date);
		tbuff->modtime = timestamp;
		tbuff->actime = timestamp;
		if(utime(files[i], tbuff) == -1) {
			printf("Error in restoring timestamp.");
			exit(-1);
		}
		free(tbuff);
		lseek(fd,SARMAG,SEEK_SET);
	}
	free(arHeader);
}



void decodeMode(int digit){
	switch(digit){
		case 0: 
			printf("---");
			break;
		case 1: 
			printf("--x");
			break;
		case 2: 
			printf("-w-");
			break;
		case 3: 
			printf("-wx");
			break;
		case 4: 
			printf("r--");
			break;
		case 5: 
			printf("r-x");
			break;
		case 6: 
			printf("rw-");
			break;
		case 7: 
			printf("rwx");
			break;
	}
}


void paddingPrint(char* buff, int paddingSize, int realSize) {
	int i;
	for (i=0; i < paddingSize; i++) {
		printf(" ");
	}
	i = 0;
	for (i=0; i < realSize; i++) {
		printf("%c", buff[i]);
	}
}

int handleHeaderName(char* buff, char* src, int size) {
	memset(buff, ' ', size);
	sprintf(buff, "%.*s", size-1, src);
	int i = 15;
	while ( i > 0) {
		if (buff[i] == '/') {
			buff[i] = '\0';
			break;
		}
		i--;
	}
	return i; //return the realSize
}


void showDetailfiles(int fd) {
	Header *arHeader = malloc(sizeof(Header));
	char buff[16];
	char* pt;
	char* date;
	while(readNext(fd, arHeader) == sizeof(Header)) {
		// print mode
		mode_t mode = (mode_t) strtoll(arHeader->ar_mode, &pt, 8);
		mode = mode % 512; // check the first digit
		decodeMode(mode/64);
		mode = mode % 64;
		decodeMode(mode/8); // check the second digit
		decodeMode(mode % 8); // check the last digit

		int size;
		// print two ids gid and uid
		size = handleString(buff, arHeader->ar_uid, 6);
		paddingPrint(buff, 6-size, size);		

		printf("/");
		size = handleString(buff, arHeader->ar_gid, 6);
		paddingPrint(buff, 0, size);

		// print file size
		size = handleString(buff, arHeader->ar_size, 10);
		paddingPrint(buff, 10-size, size);

		// print date 
		time_t timestamp = (time_t) atoi(arHeader->ar_date);
		date = ctime(&timestamp);
		paddingPrint(date, 1, 24);

		// print filename
		handleHeaderName(buff, arHeader->ar_name, sizeof(arHeader->ar_name)); // sizeof works for fixed size arrary
		printf(" %s\n", buff);
		int sizeOfContent = atoi(arHeader->ar_size);
		if (sizeOfContent%2==1) sizeOfContent++;
		lseek(fd, sizeOfContent, SEEK_CUR);
	}
	free(arHeader);
}



void showfiles(int fd) {
	Header *arHeader = malloc(sizeof(Header));
	char buff[16];
	char* ptr;
	while(readNext(fd, arHeader) == sizeof(Header)) {
		// printf("Original: %s\n", arHeader->ar_name);
		handleHeaderName(buff, arHeader->ar_name, sizeof(arHeader->ar_name)); // sizeof works for fixed size arrary
		printf("%s\n", buff);
		int sizeOfContent = atoi(arHeader->ar_size);
		if (sizeOfContent%2==1) sizeOfContent++;
		lseek(fd, sizeOfContent, SEEK_CUR);
	}
	free(arHeader);
}


int append(int fd, char* file) {
	struct stat* fileInfo = (struct stat*) malloc(sizeof(struct stat));
	Header* arHeader = (Header*) malloc(sizeof(Header));

	// check whether the file exists; yes-> append; no -> exit
	if (open(file, O_RDONLY) == -1) {
		free(fileInfo);
		free(arHeader);
		perror("Error openning the file.\n");
		exit(-1);
	}

	// status of the file is stored in fileInfo 
	if (stat(file, fileInfo) == -1) {
		perror("Error reading the status.\n");
		free(fileInfo);
		free(arHeader);
		exit(-1);
	}

	// deal with filename
	char filename[16];
	memset(filename, ' ', 16);
	// printf("The shown length of the input file: %lu\n", strlen(file)); 
	strcpy(filename, file);
	filename[strlen(file)] = '/'; // turn ending char into space
	// printf("stored info: %s\n", filename);
	
	// fill informatin into the arHeader
	strcpy(arHeader->ar_name, filename); //because this filename definitely has a ending character

	// char buff[16];

	// ar_date
	// memset(buff, ' ', 16);
	// sprintf(buff, "%ld", fileInfo->st_mtime); 
	// strncpy(arHeader->ar_date, buff, sizeof(arHeader->ar_date));
	sprintf(arHeader->ar_date, "%-12ld", fileInfo->st_mtime);
	sprintf(arHeader->ar_uid,"%-6u", fileInfo->st_uid);
	sprintf(arHeader->ar_gid,"%-6u", fileInfo->st_gid);
	
	sprintf(arHeader->ar_mode,"%-8o",fileInfo->st_mode);
	sprintf(arHeader->ar_size,"%-10lld",fileInfo->st_size);
	sprintf(arHeader->ar_fmag,"%-2s",ARFMAG);

	// write header
	int n_read = write(fd, arHeader, sizeof(Header)); // sizeof(Header) != sizeof(arHeader); sizeof(arHeader is small because it has /0 in the middle)
	if( n_read != sizeof(Header)) {
		perror("Error writing the header.");
		exit(-1);
	}

	// write the content 
	int fd_file = open(file, O_RDONLY);
	int sizeOfContent = (int)fileInfo->st_blksize;
	char fbuff[sizeOfContent];
	while ((n_read=read(fd_file, fbuff, sizeOfContent)) > 0) {
		if (write(fd, fbuff, n_read) != n_read) {
			perror("Error writing the file.");
			exit(-1);
		}
		if ((lseek(fd, 0, SEEK_END) % 2) == 1) write(fd, "\n", 1);
	}

	free(fileInfo);
	free(arHeader);

	return 0;
}


void appendFiles(int fd, char** files, int numOfFiles) {
	int i;
	for (i=0; i<numOfFiles; i++) {
		printf("This file checked: %s\n", files[i]);
		append(fd, files[i]);
	}
}


void appendAll(int fd, char* selfName) {
	DIR *dir;
    struct dirent *direntP;

    if ((dir = opendir (".")) == NULL) {
        perror ("Cannot open the current directory.");
        exit (1);
    }

    while ((direntP = readdir(dir)) != NULL) {
    	if (direntP->d_type == DT_REG) {
    		struct stat *fstat = (struct stat*) malloc(sizeof(struct stat));
			stat(direntP->d_name, fstat);
			// if(stat(direntP->d_name,fstat)==-1){
			// 	printf("%s\n", direntP->d_name);
			// 	continue;
			// }
    		if ( difftime(time(NULL), fstat->st_mtime) <= 7200 ) {
    			if ((strncmp(direntP->d_name, selfName, strlen(selfName))) != 0) {
    				printf ("[%s]\n", direntP->d_name);
    				// printf("[%s]\n", selfName);
    				append(fd, direntP->d_name);
    			}
    		}
    	}
    }

}




void deleteFile(int fd, char* archiveFile, char* file) {
	Header *arHeader = malloc(sizeof(Header));

	// get informatin of the archiveFile
	struct stat *f_stat = malloc(sizeof(struct stat));
	fstat(fd, f_stat);
	int buf_size = (int)f_stat->st_blksize;

	// create a new archiveFile and unlink 
	int newfd;
	unlink(archiveFile);
	if((newfd=creat(archiveFile, 0666))==-1) {
		perror("Error in creating the archiveFile.");
		exit(-1);
	}
	if(write(newfd, ARMAG, SARMAG) == -1) {
		perror("Error in writing the archiveFile.");
		exit(-1);
	}

	// variables used by while loop
	char fname[sizeof(arHeader->ar_name)];
	int fileSize;
	int find = -1;
	char buff[buf_size];
	while(readNext(fd, arHeader) != -1) {
		// handle the size of the matched file
		fileSize = (int) atoi(arHeader->ar_size);		
		if (fileSize%2==1) fileSize++; // because multiple of 2 is required for each memory space
		handleString(fname, arHeader->ar_name, sizeof(arHeader->ar_name));

		// if matched, continue (skipping the writing of the file)
		if ((strncmp(fname, file, strlen(file)) == 0) && (find == -1)) {
			find = 0;
			printf("FOUND AND DELETE THE FILE. %s\n", file);
			lseek(fd, fileSize, SEEK_CUR);
			continue;
		}

		// write the arHeader to the new archiveFile
		if (write(newfd, arHeader, sizeof(Header)) == -1) {
			perror("Error writing the header of archiveFile.");
			exit(-1);
		}

		int nread;
		while (fileSize > 0) {
			if (fileSize < buf_size) {
				buf_size = fileSize;
			}
			if ((nread=read(fd, buff, buf_size)) == buf_size) {
				if(write(newfd, buff, buf_size) != buf_size) {
					perror("Error writing the content of the archiveFile.");
					exit(-1);
				}
			} 
			fileSize -= buf_size;
		}

	}

	if (find == -1) {
		printf("File %s not found under the archive.\n", file);
	} 

	free(f_stat);
	free(arHeader);
}


void delete(int fd, char* archiveFile, char** files, int numOfFiles) {
	int i;
	for (i=0; i < numOfFiles; i++) {
		deleteFile(fd, archiveFile, files[i]);
	}
}


int main(int argc, char *argv[]) {
	if (argc < 3) {
		printUsage();
		exit(-1); 
	}

	char option = argv[1][0];
	char* archiveFile = argv[2];
	char* files[argc-3];
	if (argc > 3) {
		for (int i=0; i < argc-3; i++) {
			files[i] = argv[i+3];
		}
	}

	// TODO: create an archive if it does not exist 
	int fd;
	// TODO: pre-check the file/archiveFile existence and determine fd 
	if (access( archiveFile, F_OK) != -1) {
		// open for use 
		fd = open(archiveFile, O_APPEND | O_RDWR);
		char magic[9];
		magic[8] = '\0';
		read(fd, magic, 8);
		if(strcmp(magic, ARMAG) != 0){
			printf("Error: File not recognized\n");
			close(fd); 
			exit(-1);
		}
	} else {
		if (option == 'q' || option == 'A') {
			fd = open(archiveFile, O_RDWR|O_CREAT, 0666);
			write(fd, ARMAG, 8);
		} else {
			printf("This archive file does not exist.\n");
			exit(-1);
		}

	}


	switch(option) {
		case 'q':
			appendFiles(fd, files, argc-3);
			break;
		case 'x':
			extract(fd, files, argc-3);
			break;
		case 't':
			showfiles(fd);
			break;
		case 'v':
			showDetailfiles(fd);
			break;
		case 'd':
			delete(fd, archiveFile, files, argc-3);
			break;
		case 'A':
			appendAll(fd, archiveFile);
			break;
		default:
			printf("wrong option %c; please look up the following instructions:\n", option);
			printUsage();
	}

	close(fd);
	exit(0);
}


void printUsage(){
	printf("usage:\n");
	printf("q\tquickly append named files to archive\n");
	printf("x\textract named fiels\n");
	printf("t\tprint a concise table of contents of the archive\n");
	printf("v\tprint a verbose table of contents of the archive\n");
	printf("d\tdelete named files from archive\n");
	printf("A\tquickly append all ordinary files in the current directory that have been modified within the last two hours (except the archive itself).\n");

}





















