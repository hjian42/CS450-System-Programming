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


void print_mode(int digit){
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

// understand why header structs are continuous here???
int next_file(int fd, Header* my_hdr){
	int read_size=read(fd,my_hdr,sizeof(Header));
	if(read_size==-1 || read_size!=sizeof(Header)) return -1;
	else return read_size;
}

// convert permission
void permission(mode_t mode){
	mode = mode % 512;
	print_mode(mode/64);
	mode = mode % 64;
	print_mode(mode/8);
	print_mode(mode%8);
	printf("\t");
}


// build str; improvement of sprintf
char*  str(char* buf, char* src,int size,int header){
	int i = 0;
	int j = 0;
	while (i<size){
		if (src[i]=='\0'||src[i]=='/'){
			buf[i] = '\0';
			break;
		} else if (src[i] == '\n') {
			i++;
			continue;
		}
		buf[j++]=src[i++];
	}
	if (header==1 || buf[i-1] != '/') buf[i++]='/';
	return buf;
}




void listFiles(int fd, int verbosity) {
	Header * file_header = malloc(sizeof(Header));

	char buf[16];
	char* ptr;
	char* tm;
	while(next_file(fd,file_header)!=-1){
		if(verbosity==1) {
			mode_t mode=(mode_t) strtoll(file_header->ar_mode,&ptr,8);
			permission(mode);
			// str(buf,file_header->ar_uid,6,0);
			// nospace(buf);
			printf("%.6s", file_header->ar_uid);
			printf("/");
			printf("%.6s",file_header->ar_gid );
			printf("%.10s",file_header->ar_size );
			time_t timestamp = (time_t) atoi(file_header->ar_date);
			tm=ctime(&timestamp);
			printf("%.24s ",tm);
		}
		str(buf,file_header->ar_name,15,0);
		printf("%s\n",buf);
		int contentSize=atoi(file_header->ar_size); 
		lseek(fd,contentSize,SEEK_CUR);
	}
	free(file_header);

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

	char buff[17];

	// fill informatin into the arHeader
	// ar_name 
	// str(arHeader->ar_name, filename, 16, 1);
	// memset(arHeader->ar_name, '\0', sizeof(arHeader->ar_name));
	strcpy(arHeader->ar_name, filename);

	// ar_date
	// memset(arHeader->ar_date, ' ', sizeof(arHeader->ar_date));
	sprintf(buff, "%ld", fileInfo->st_mtime);
	strcpy(arHeader->ar_date, buff);
	// str(arHeader->ar_date, buff, 12, 0);

	// ar_uid
	// memset(arHeader->ar_uid, ' ', sizeof(arHeader->ar_uid));
	sprintf(buff, "%ld", (long) fileInfo->st_uid);
	strcpy(arHeader->ar_uid, buff);
	// str(arHeader->ar_uid, buff, 6, 0);

	// ar_gid
	// memset(arHeader->ar_gid, ' ', sizeof(arHeader->ar_gid));
	sprintf(buff, "%ld", (long) fileInfo->st_gid);
	strcpy(arHeader->ar_gid, buff);
	// str(arHeader->ar_gid, buff, 6, 0);

	// ar_mode
	// memset(arHeader->ar_mode, ' ', sizeof(arHeader->ar_mode));
	sprintf(buff, "%o", fileInfo->st_mode);
	strcpy(arHeader->ar_mode, buff);
	// str(arHeader->ar_mode, buff, 8, 0);

	// ar_size
	// memset(arHeader->ar_size, ' ', sizeof(arHeader->ar_size));
	sprintf(buff, "%lld", fileInfo->st_size);
	strcpy(arHeader->ar_size, buff);
	// str(arHeader->ar_size, buff, 10, 0);

	// ar_fmag
	// memset(arHeader->ar_fmag, ' ', sizeof(arHeader->ar_fmag));
	strcpy(arHeader->ar_fmag, ARFMAG);
	// str(arHeader->ar_fmag,ARFMAG,2,0);

	// write arHeader to the archiveFile
	// lseek(fd, 0, SEEK_END);

	// write header
	int n_read = write(fd, (char*) arHeader, sizeof(Header));
	if( n_read != sizeof(Header)) {
		perror("Error writing the header.");
		exit(-1);
	}

	// write the content 
	int fd_file = open(filename, O_RDONLY);
	char fileBuff[fileInfo->st_blksize];
	while ((n_read=read(fd_file, fileBuff, fileInfo->st_blksize)) > 0) {
		if (write(fd, fileBuff, n_read) != n_read) {
			perror("Error writing the file.");
			exit(-1);
		}
		if ((lseek(fd, 0, SEEK_END) % 2) == 1) write(fd, "\n", 1);
	}
	// write(fd, "\n", 1);
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


// what is fd in C, an int or ???
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




void extract(int fd, char* archiveFile, char** files, int numOfFiles) {
	Header *arHeader = malloc(sizeof(Header));
	int i;
	// printf("%d\n", fd);
	// printf("%d\n", numOfFiles);
	// printf("Welcome to extract PART.\n");

	char to[sizeof(arHeader->ar_name)];
	int find;
	int fileSize;

	for (i=0; i< numOfFiles; i++) {
		find=-1;
		while (next_file(fd, arHeader) != -1) {
			str(to, arHeader->ar_name, sizeof(arHeader->ar_name), 0);
			fileSize = (int) atoi(arHeader->ar_size);
			if (strncmp(to, files[i], strlen(files[i])) == 0) {
				find = 0;
				break;
			} else {
				lseek(fd, fileSize, SEEK_CUR);
			}

			// printf("The file size of i is : %d\n", fileSize);
			// printf("%s\n", to);

		}
		if (find == -1) {
			printf("myar: %s: Not found in archive\n",files[i]);
			continue;
		}

		// if found
		char* ptr;
		// follow original mode
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
		time_t timestamp = atoll (arHeader->ar_date);
		struct utimbuf* timebuf=(struct utimbuf*) malloc(sizeof(struct utimbuf));		timebuf->actime = timestamp;
		timebuf->modtime = timestamp;
		if(utime(files[i], timebuf) == -1) {
			printf("Error in restoring timestamp.");
			exit(-1);
		}
		free(timebuf);
		lseek(fd,SARMAG,SEEK_SET);
	}
	free(arHeader);
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
			printf("Error: File not recorgnized\n");
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
			extract(fd, archiveFile, files, argc-3);
			break;
		case 't':
			listFiles(fd, 0);
			break;
		case 'v':
			listFiles(fd, 1);
			break;
		case 'd':
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





















