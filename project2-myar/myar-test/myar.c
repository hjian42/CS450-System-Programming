// THIS CODE IS MY OWN WORK, IT WAS WRITTEN WITHOUT CONSULTING A TUTOR OR CODE WRITTEN BY OTHER STUDENTS - Jiayu Yao


// CS450 HW2 excutable command q/x/t/A
#include <stdio.h>
#include <stdlib.h>
#include <ar.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <utime.h>
#include <time.h>
#include <errno.h>
#include <dirent.h>

size_t buf_size; 
typedef struct ar_hdr Header;

// Determine whether the file is an archive by checking the magic number
int archive_check(int fd){
	char buf[SARMAG];
	read(fd,buf,SARMAG);
	if(strncmp(buf,ARMAG,SARMAG)!=0){
		lseek(fd,-1*SARMAG,SEEK_CUR);
		return -1;
	}
	return 0;
}

// Check if file exists
int exist(char* file_name){
	int fd;
	struct stat* status=(struct stat*) malloc(sizeof(struct stat));
	if(stat(file_name,status)==-1){
		perror("myar");
		return -1;
	}
	buf_size=status->st_blksize;
	if((fd=open(file_name,O_RDWR))==-1){
		perror("myar");
		return -1;
	}
	free(status);
	return fd;
}

// Find next file header
int next_file(int fd, Header* my_hdr){
	int read_size=read(fd,my_hdr,sizeof(Header));
	if(read_size==-1 || read_size!=sizeof(Header)) return -1;
	else return read_size;
}

// build str
char*  str(char* buf, char* src,int size,int header){
	int i;
	memset(buf,' ',size);
	for(i=0;i<size;i++){
		if (src[i]=='\0'||src[i]=='/'){
			break;
		}
		buf[i]=src[i];
	}
	if (header==1) buf[i++]='/';
	return buf;
}

// create archive
int creat_file(char* file, Header* my_hdr){
	char* ptr;
	mode_t mode=(mode_t) strtol(my_hdr->ar_mode,&ptr,8);
	return creat(file,mode);
}
// append a file to an archive
int append(int fd,char* file){
	struct stat* status=(struct stat*) malloc(sizeof(struct stat));
	Header* arhd=(Header*) malloc(sizeof(Header));

	if((open(file,O_RDWR))==-1){
		str(arhd->ar_name,file,16,0);
		if(write(fd,(char*)arhd,sizeof(Header))!=sizeof(Header)){
			perror("write header error");
			return -1;
		}
		free(status);
		free(arhd);
		perror("Error opening file but still apppend");
		return -1;
	}
	if(stat(file,status)==-1){
		perror("Error reading status");
		free(status);
		free(arhd);
		return -1;
	}
	
	char strbuf[16];
	
	// Create header
	str(arhd->ar_name,file,16,1);
	
	sprintf(strbuf,"%ld",status->st_mtime);
	str(arhd->ar_date,strbuf,12,0);

	sprintf(strbuf,"%u",status->st_uid);
	str(arhd->ar_uid,strbuf,6,0);

	sprintf(strbuf,"%d",status->st_gid);
	str(arhd->ar_gid,strbuf,6,0);

	sprintf(strbuf,"%o",status->st_mode);
	str(arhd->ar_mode,strbuf,8,0);

	sprintf(strbuf,"%lld",status->st_size);
	str(arhd->ar_size,strbuf,10,0);

	str(arhd->ar_fmag,ARFMAG,2,0);

	ssize_t nread;
	// Write header
	lseek(fd, 0, SEEK_END);
	if(write(fd,(char*)arhd,sizeof(Header))!=sizeof(Header)){
		perror("write header error");
		return -1;
	}
	int from_fd=exist(file);
	char buf[buf_size];
	while((nread=read(from_fd,buf,buf_size))>0){
		if(write(fd,buf,nread)!=nread){
			perror("write file content error");
			return -1;
		}
		if ((lseek(fd, 0, SEEK_END) % 2) == 1) write(fd, "\n", 1);
	}

	free(status);
	free(arhd);

	return 0;

	
}
// quick append
int quick_append(char* afile, char** files,int size){
	// Create an archive if does not exist
	int fd;
	if((fd=open(afile,O_RDWR))==-1){
		printf("ar: creating archive %s\n",afile);
		if((fd=creat(afile,0666))==-1) perror("Error creating archive");
		if(write(fd,ARMAG,SARMAG)==-1) perror("Error writing archive header");
	}
	else{
		if (archive_check(fd)==-1){
			printf("myar: %s: Inapporiate file or format\n",afile);
			return -1;
		}
	}
	int i;
	for(i=0;i<size;i++){
		append(fd,files[i]);
	}
	return 0;
}
// extract files from archive
int extract(char* afile,char** files,int size){
	int fd=exist(afile);
	if(fd==-1) return -1;
	if(archive_check(fd)==-1){
		printf("myar: %s: Inapporiate file or format\n",afile);
		return -1;
	}
	Header* my_hdr=(Header*) malloc(sizeof(Header));
	char to[sizeof(my_hdr->ar_name)];
	int i;
	int find;
	ssize_t file_size;
	for (i=0;i<size;i++){
		// Find the file in the archive
		find=-1;
		while(next_file(fd,my_hdr)!=-1){
			str(to,my_hdr->ar_name,sizeof(my_hdr->ar_name),0);
			file_size=(ssize_t) atoll(my_hdr->ar_size);
			if(strncmp(to,files[i],strlen(files[i]))==0){
				find=0;
				break;
			}else{
				lseek(fd,file_size,SEEK_CUR);
			}
		}
		if (find==-1){
			printf("%x\n",i );
			printf("myar: %s: Not found in archive\n",files[i]);
			continue;
		}
		// if found, write our file content
		int to_fd;
		to_fd=creat_file(files[i],my_hdr);
		ssize_t nread;
		while(file_size>0U){
			if (file_size<buf_size) buf_size=file_size;
			char* buf[buf_size];
			if((nread=read(fd,buf,buf_size))>0){
				if(write(to_fd,buf,nread)!=nread){
					perror("write error");
					return -1;
				}
			}
			file_size=file_size-buf_size;
		}
		//Restore timestamp
		time_t timestamp=atoll(my_hdr->ar_date);
		struct utimbuf* timebuf=(struct utimbuf*) malloc(sizeof(struct utimbuf));
		timebuf->actime=timestamp;
		timebuf->modtime=timestamp;
		if(utime(files[i],timebuf)==-1) {
			printf("Error in restoring timestamp");
			return -1;
		}
		free(timebuf);
		lseek(fd,SARMAG,SEEK_SET);
	}
	free(my_hdr);
	return 0;
}
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
// convert permission
void permission(mode_t mode){
	mode = mode % 512;
	print_mode(mode/64);
	mode = mode % 64;
	print_mode(mode/8);
	print_mode(mode%8);
	printf("     ");
}
void nospace(char* s){
	int i=0;
	while(s[i]!=' '){
		printf("%c",s[i]);
		i++;
	}
}
// print information table
int print(char* afile,int verbosity){
	int fd=exist(afile);
	if (fd==-1) return -1;

	if(archive_check(fd)==-1){
		printf("myar: %s: Inapporiate file or format\n",afile);
		return -1;
	}
	
	Header* my_hdr=(Header*) malloc(sizeof(Header));
	char buf[16];
	char* ptr;
	char* tm;
	while(next_file(fd,my_hdr)!=-1){
		if(verbosity==1){
			mode_t mode=(mode_t) strtoll(my_hdr->ar_mode,&ptr,8);
			permission(mode);
			str(buf,my_hdr->ar_uid,6,0);
			nospace(buf);
			printf("/");
			printf("%.6s",my_hdr->ar_gid );
			printf("%.10s",my_hdr->ar_size );
			time_t timestamp = (time_t) atoi(my_hdr->ar_date);
			tm=ctime(&timestamp);
			printf("%.24s ",tm);
		}
		str(buf,my_hdr->ar_name,16,0);
		printf("%s",buf);
		printf("\n");
		ssize_t file_size=atoll(my_hdr->ar_size);
		lseek(fd,file_size,SEEK_CUR);
	}
	free(my_hdr);
	return 0;
}
// Append ordinary file in the directory
int append_ordinary(char* afile){
	int fd;
	if((fd=creat(afile,0666))==-1) perror("Error creating archive");
	if(write(fd,ARMAG,SARMAG)==-1) perror("Error writing archive header");

	
	DIR* dir_stream;
	struct dirent* entries=malloc(sizeof(struct dirent));
	struct stat* status=malloc(sizeof(struct stat));

	if(!(dir_stream= opendir("."))){
		perror("Error opening archive");
		return -1;
	}
	while((entries=readdir(dir_stream))!=NULL){
		// Skip archive itself
		if(strncmp(entries->d_name,afile,strlen(afile))==0) continue;
		char file_name[16];
		str(file_name,entries->d_name,16,0);
		if(stat(entries->d_name,status)==-1){
			perror("Error accessing file status");
			continue;
		}
		// Append file modified in an hour
		time_t cur_time;
		cur_time = time(NULL);
		if(cur_time-status->st_mtime>3600) continue;
		// Skip non-regular files
		if(S_ISREG(status->st_mode)){
			append(fd,entries->d_name);
		}
	}
	free(entries);
	free(status);
	return 0;
}

int deletefile(char* afile,char* file){
	int fd=exist(afile);
	if(fd==-1) return -1;
	if(archive_check(fd)==-1){
		printf("myar: %s: Inapporiate file or format\n",afile);
		return -1;
	}
	int a_fd;
	Header* my_hdr=(Header*) malloc(sizeof(Header));
	char* buf[buf_size];
	char to[sizeof(my_hdr->ar_name)];
	int i;
	int find;
	ssize_t file_size;
	ssize_t nread;
	unlink(afile);
	if((a_fd=creat(afile,0666))==-1) perror("Error creating archive");
	if(write(a_fd,ARMAG,SARMAG)==-1) perror("Error writing archive header");
	// Find the file in the archive
	find=-1;
	while(next_file(fd,my_hdr)!=-1){
		str(to,my_hdr->ar_name,sizeof(my_hdr->ar_name),0);
		file_size=(ssize_t) atoll(my_hdr->ar_size);
		if (file_size%2==1) file_size++;
		if(find!=0 && strncmp(to,file,strlen(file))==0){
			find=0;
			lseek(fd,file_size,SEEK_CUR);
			continue;
		}
		if(write(a_fd,my_hdr,60)==-1){perror("Error writing header");}
		while(file_size>0U){
			if (file_size<buf_size) buf_size=file_size;
			if((nread=read(fd,buf,buf_size))>0){
				if(write(a_fd,buf,nread)!=nread){
					perror("write error");
					return -1;
				}
			}
			file_size=file_size-buf_size;
		}
	}
	if (find==-1){
		printf("myar: %s: Not found in archive\n",file);
	}
	free(my_hdr);
	return 0;

}
// delete files from archive
int delete(char* afile,char** files,int size){
	int i ;
	for(i=0;i<size;i++){
		deletefile(afile,files[i]);
	}
	return 0;
}
// print usages
void usage(){
	printf("Usage:  myar q quickly append named files to archive\n");
	printf("\tmyar x extract named files\n");
	printf("\tmyar t print a concise table of contents of the archive\n");
	printf("\tmyar A quickly append all ordinary files in the currernt directory that have been modified within the last hour(except the archive itself)\n");
}
// main method
int main(int argc, char *argv[]) {
	if(argc<3){
		usage();
		return -1;
	}
	char key = argv[1][0];
	char* afile=argv[2];
	char* files[argc-3];
	if(argc>3){
		int i ;
		for(i=0; i < argc-3;i++) {files[i]=argv[3+i];}
	};
	
	//Check validity of the argumen
	switch(key){
		case 'q':
			quick_append(afile,files,argc-3);
			break;
		case 'x':
			extract(afile,files,argc-3);
			break;
		case 't':
			print(afile,0);
			break;
		case 'v':
			print(afile,1);
			break;

		case 'd':
			delete(afile,files,argc-3);
			break;
		case 'A':
			append_ordinary(afile);
			break;
		default:
			printf("illegal option %c\n",key );
			usage();
	}
}