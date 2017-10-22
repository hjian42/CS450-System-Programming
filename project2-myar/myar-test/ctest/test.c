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

typedef struct ar_hdr Header;

int main()
{   
    // char* a = "Hello World!";
    // char pt[20];
    // strcpy(pt, a);
    // printf("%s\n", pt);
    char* targetFile = "t1";
    int fd = open("right.a", O_RDWR);
    int nread;
    // Header *arHeader = malloc(sizeof(Header));
    char buff[17];
    lseek(fd, 8, SEEK_SET);


    // memset(buff, NULL, sizeof(buff));
    while ((nread=read(fd, buff, sizeof(Header))) == sizeof(Header)) {
    	buff[nread] = '\0';
    	printf("READS: %d\n", nread);
    	printf("%s\n", buff);

    	// int file_size = (int)atoi(buff->ar_size);
    	// printf("%d\n", file_size);
    	// memset(buff, NULL, sizeof(buff));
    }



    return 0;
}
