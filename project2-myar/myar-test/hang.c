// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Hang Jiang

// command: myar option archiveName files
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
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>

void printUsage();
void listFiles(int fd, int verbosity); // 0 for t; 1 for v

size_t buf_size; 
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
		} 
		i++;
	}
	return i;
}

void extract(int fd, char** files, int numOfFiles) {
	Header *arHeader = malloc(sizeof(Header));
	
	// printf("%d\n", fd);
	// printf("%d\n", numOfFiles);
	// printf("Welcome to extract PART.\n");
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
			int lengOfName = handleString(fbuff, arHeader->ar_name, sizeof(arHeader->ar_name));
			fileSize = (int) atoi(arHeader->ar_size);
			// printf("%s\n", fbuff);
			// printf("%s\n", files[i]);
			// printf("length of file is %d\n", lengOfName);
			// checkpoint: 
			// printf("%d\n", strncmp(fbuff, files[i], lengOfName));
			if (strncmp(fbuff, files[i], lengOfName) == 0) {
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
		// printf("MODE: %s\n", arHeader->ar_mode);
		mode_t mode = (mode_t) strtol(arHeader->ar_mode, &ptr, 8);
		// printf("MODE 2: %hu\n", mode);
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



// void listFiles(int fd, int verbosity) {
// 	Header * file_header = malloc(sizeof(Header));

// 	char buf[16];
// 	char* ptr;
// 	char* tm;
// 	while(next_file(fd,file_header)!=-1){
// 		if(verbosity==1) {
// 			mode_t mode=(mode_t) strtoll(file_header->ar_mode,&ptr,8);
// 			permission(mode);
// 			// str(buf,file_header->ar_uid,6,0);
// 			// nospace(buf);
// 			printf("%.6s", file_header->ar_uid);
// 			printf("/");
// 			printf("%.6s",file_header->ar_gid );
// 			printf("%.10s",file_header->ar_size );
// 			time_t timestamp = (time_t) atoi(file_header->ar_date);
// 			tm=ctime(&timestamp);
// 			printf("%.24s ",tm);
// 		}
// 		str(buf,file_header->ar_name,15,0);
// 		printf("%s\n",buf);
// 		int contentSize=atoi(file_header->ar_size); 
// 		lseek(fd,contentSize,SEEK_CUR);
// 	}
// 	free(file_header);

// }

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
		handleString(buff, arHeader->ar_name, sizeof(arHeader->ar_name)); // sizeof works for fixed size arrary
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
		
		handleString(buff, arHeader->ar_name, sizeof(arHeader->ar_name)); // sizeof works for fixed size arrary
		printf("%s\n", buff);
		int sizeOfContent = atoi(arHeader->ar_size);
		if (sizeOfContent%2==1) sizeOfContent++;
		lseek(fd, sizeOfContent, SEEK_CUR);
	}
	free(arHeader);
}




int append(int fd, char* filename) {
	struct stat* fileInfo = (struct stat*) malloc(sizeof(struct stat));
	Header* arHeader = (Header*) malloc(sizeof(Header));

	// check whether the file exists; yes-> append; no -> exit
	if (open(filename, O_RDWR) == -1) {
		free(fileInfo);
		free(arHeader);
		perror("Error openning the file.\n");
		exit(-1);
	}

	// status of the file is stored in fileInfo 
	if (stat(filename, fileInfo) == -1) {
		perror("Error reading the status.\n");
		free(fileInfo);
		free(arHeader);
		exit(-1);
	}

	char buff[16];
	// memset(buff, '\0', 16);

	// fill informatin into the arHeader
	// good things about them: stat() function handles the string ending for each variable inside
	// our arHeader didn't because we brutely read it wtihout handling those strings well
	// so we dont need to use stringHandle even
	// ar_name 
	strcpy(arHeader->ar_name, filename); //because this filename definitely has a ending character

	// ar_date
	sprintf(buff, "%ld", fileInfo->st_mtime); 
	strcpy(arHeader->ar_date, buff);

	// ar_uid
	sprintf(buff, "%ld", (long) fileInfo->st_uid);
	strcpy(arHeader->ar_uid, buff);

	// ar_gid
	sprintf(buff, "%ld", (long) fileInfo->st_gid);
	strcpy(arHeader->ar_gid, buff);

	// ar_mode
	sprintf(buff, "%o", fileInfo->st_mode);
	strcpy(arHeader->ar_mode, buff);
	// str(arHeader->ar_mode, buff, 8, 0);

	// ar_size
	sprintf(buff, "%lld", fileInfo->st_size);
	strcpy(arHeader->ar_size, buff);
	// str(arHeader->ar_size, buff, 10, 0);

	// ar_fmag
	strcpy(arHeader->ar_fmag, ARFMAG);
	// str(arHeader->ar_fmag,ARFMAG,2,0);

	// write header
	int n_read = write(fd, arHeader, sizeof(Header)); // sizeof(Header) != sizeof(arHeader); sizeof(arHeader is small because it has /0 in the middle)
	if( n_read != sizeof(Header)) {
		perror("Error writing the header.");
		exit(-1);
	}

	// write the content 
	int fd_file = open(filename, O_RDONLY);
	int sizeOfContent = (int)fileInfo->st_blksize;
	char fbuff[sizeOfContent];
	while ((n_read=read(fd_file, fbuff, sizeOfContent)) > 0) {
		// printf("%d\n", n_read);
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


// I cannot use char*[] here, values do not automatically copy to the new array???
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
    		if ( difftime(time(NULL), fstat->st_mtime) <= 7200 ) {
    			if (strcmp(direntP->d_name, selfName) != 0) {
    				printf ("[%s]\n", direntP->d_name);
    				append(fd, direntP->d_name);
    			}
    			// break;
    		}
    	}
    }

}




void deleteFile(int fd, char* archiveFile, char* file) {
	printf("WORKING on: %s\n", file);
	Header *arHeader = malloc(sizeof(Header));

	// get informatin of the archiveFile
	struct stat *f_stat = malloc(sizeof(struct stat));
	fstat(fd, f_stat);
	int buf_size = (int)f_stat->st_blksize;

	// create a new archiveFile and unlink 
	int newfd;
	// unlink(archiveFile);
	if((newfd=creat("new.a", 0666))==-1) {
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
		// print Eachfile information from arHeader
		// printf("%.16s\n", arHeader->ar_name);
		// printf("the size of the file is %lu\n", sizeof(arHeader->ar_name));
		// handle the size of the matched file
		fileSize = (int) atoi(arHeader->ar_size);		
		if (fileSize%2==1) fileSize++; // because multiple of 2 is required for each memory space
		handleString(fname, arHeader->ar_name, sizeof(arHeader->ar_name));
		// printf("Targeted value is : %s, the size is %lu. \n", file, sizeof(file));
		// printf("Returned value is : %s, the size is %lu. \n", fname, sizeof(fname));

		// if matched, continue (skipping the writing of the file)
		// fname contains many NULL terminators while file only has one NULL terminator
		// strlen does not count NULL terminator
		if ((strncmp(fname, file, strlen(file)) == 0) && (find == -1)) {
			find = 0;
			printf("FOUND THE FILE. %s\n", file);
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
		exit(-1); // 1 or -1?
	}

	// printf("This is a test.\n");

	char option = argv[1][0];
	char* archiveFile = argv[2];
	char* files[argc-3];
	if (argc > 3) {
		for (int i=0; i < argc-3; i++) {
			files[i] = argv[i+3];
			// printf("%s\n", files[i]);
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
			close(fd); // why do we have to close it? 
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
			printf("doing delete command\n");
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





















