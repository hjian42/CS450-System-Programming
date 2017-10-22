/* THIS CODE IS MY OWN WORK, IT IS WRITTEN WITHOUT CONSULTING A TUTOR OR CODE WRITTEN BY OTHER STUDENT -YIYANG ZHAO*/
// All option attempted: q A t v d x
// Please use myar [option] [arname] {file_names[]}; do not put dash (-) before options
// Ex: myar q t.ar file1 file2...    NOT myar -q ...

/*include*/
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <utime.h>
#include <time.h>
#include <ar.h>
#include <fcntl.h>
#include <stdio.h>
#include <dirent.h>

//function decs
//#include <myarheader.h>
void ar_list(int ar_fd, int verbosity);
void ar_ext(int ar_fd, int argc, char **argv);
void ar_app(int ar_fd, int argc, char **argv);
void ar_appall(int ar_fd, int argc, char **argv);
void ar_del(int ar_fd, int argc, char **argv);

//main
void print_usage()
{
		printf("Usage: myar key afile name...\n");
		printf("q	quickly append named files to archive\n");
		printf("x	extract named files\n");
		printf("t	print a concise table of contents of the archive\n");
		printf("v	print a verbose table of contents of the archive\n");
		printf("d	delete named files from archive\n");
		printf("A	quickly append all ordinary files in the current directory that have been modified within the last hour except the archive itself\n");
}

int main (int argc, char **argv)
{
	//myar key afile name
	/*
 	q app [named]
	x ext [named]
	t prt(cons)
	v prt(vbo)
	d del [named]
	A app all(mod'd)
	*/
	char option;
	char* ar_filename;
	char* op_filename;
	int ar_fd;

	//argument check
	if (argc <= 2){
		print_usage();
		exit(1);
	}
	//passing arguments: option and ar file name	
	option = argv[1][0];
	ar_filename = argv[2];

	if (!(option == 'q' | option == 'x'|option=='t'|option =='v'|option =='d'|option == 'A')){
		print_usage();
		exit(1);
	}
	//Open the ar file
	
	//test if arfile exist. yes->open no&&'q'|'A'->create otherwise quit
	if ( access( ar_filename, F_OK ) != -1 ){
		//open for use
		ar_fd = open(ar_filename, O_RDWR);
		char magic[8];
		magic[8] = NULL;
		read(ar_fd, magic, 8);
		if(strcmp(magic, ARMAG) != 0){
			printf("Error: File not recorgnized\n");
			close(ar_fd);
			exit(-1);
		}
	} else{

		if (option =='q'||option == 'A'){
			ar_fd = open(ar_filename, O_RDWR|O_CREAT, 0666);
           		printf("Archive file not found; creating new archive file\n");
			write(ar_fd, ARMAG, 8);
			//create file
		} else{
			printf("Error: target archive file not exist\n");	
			exit(-1);	
		}

	}

	/* 
	t,v -> ar_list(ar_fd, int 0(t)/1(v))
	x -> ar_ext(ar_fd,argc,**argv)
	q -> ar_app(ar_fd,argc,**argv)
	A -> ar_appall(ar_fd,(cwd?)) -> ar_app
	d -> ar_del(ar_fd,argc,**argv)
	*/
	switch(option){
	
		case('t'):
			ar_list(ar_fd,0);
			break;
		case('v'):
			ar_list(ar_fd,1);
			break;
		case('q'):
			ar_app(ar_fd,argc,argv);
			break;
		case('x'):
			ar_ext(ar_fd,argc,argv);
			break;
		case('A'):
			ar_appall(ar_fd,argc,argv);
			break;

		case('d'):
			ar_del(ar_fd,argc,argv);
			break;
		default:
			exit(-1);
	}

}


void ar_list(int ar_fd, int verbosity){
	//DONE
	struct ar_hdr * file_header = malloc(sizeof(struct ar_hdr));
	
	while (read(ar_fd, file_header, sizeof(struct ar_hdr)) == sizeof(struct ar_hdr)) {
		//printf("readaaa\n");		
		int i;
		int filesize = (int)strtol(file_header->ar_size,NULL,10);		
		for (i=15;i>0;i--){
			//printf("%d\n",i);
			if (file_header->ar_name[i] == '/'){
				file_header->ar_name[i] = NULL;
				break;
			}
		}	
	
		if (!(verbosity)){
			printf("%s\n",file_header->ar_name);
		}else{

			//DONE
			// 3*3mode uid/gid   size timestamp[Oct 13 19:35 2016] filename
			int j;
			//mode			
			for (j=3;j<6;j++){
				int per = file_header->ar_mode[j] - '0';
				int perx = per % 2;
				per = per/2;
				int perw = per % 2;
				per = per/2;
				int perr = per % 2;
				if (perr) printf("r"); else printf("-");
				if (perw) printf("w"); else printf("-");
				if (perx) printf("x"); else printf("-");
			}
			//printf(" ");

			//uid/gid
			int uid = atoi(file_header->ar_uid);
			int gid = atoi(file_header->ar_gid);
			printf("%6d/%-6d ",uid, gid);
			printf("%d", filesize);
			time_t mtimestamp;	 
			mtimestamp = strtol(file_header->ar_date,NULL,10);
			char* printtime[20];
			strftime(printtime, 20, " %b %e %R %Y", localtime(&mtimestamp));
			printf("%-20s", printtime);
			printf("%-16s\n",file_header->ar_name);
		}	
		char *something_weird;	
		
		//printf("%d\n", filesize);
		if (filesize%2) filesize++;
		lseek(ar_fd,filesize,SEEK_CUR);


		
	}

	close(ar_fd);
}


void ar_ext(int ar_fd, int argc, char **argv){
	//TODO

	int i;
	struct ar_hdr * file_header = malloc(sizeof(struct ar_hdr));
	printf("%d\n",argc);
	for (i=3;i<argc;i++){
		printf("finding %s\n",argv[i]);
		//find named file
		int filesize;
		int flag = 0;
		while (read(ar_fd, file_header, sizeof(struct ar_hdr)) == sizeof(struct ar_hdr)){
			int j;			
			for (j = 16; j >=0; j--){
            			if (file_header->ar_name[j] == '/') {
               				file_header->ar_name[j] = '\0';
                 			
					break;
           		 	}
      		 	}
			filesize = (int)strtol(file_header->ar_size,NULL,10);
			
			if ((strcmp(file_header->ar_name, argv[i]) != 0)){
				//skip
				if (filesize%2) filesize++;
				lseek(ar_fd,filesize,SEEK_CUR);
				printf("going thru:%s\n",file_header->ar_name);// for (j=16; j >= 0; j--) {
			// 	break;
			// }
		
			} else{
				//break
				flag = 1;
				printf("found %s\n",file_header->ar_name);
				break;
			}
		}
	
		if (flag == 0) {
			printf("Error: file to extract not found.\n");
			exit(-1);
		}
		//create a file	
		struct utimbuf *file_utime = malloc(sizeof(struct utimbuf));
   	 	int *file_mode;
   		time_t file_time;
		
		sscanf(file_header->ar_mode, "%o", file_mode);
		file_time = (time_t)strtol(file_header->ar_date,NULL,10);
		file_utime->actime = file_time;
		file_utime->modtime = file_time;
		//extract its content write using buffer
		printf("%s\n %o\n",file_header->ar_mode,*file_mode);
		int extf_fd = creat(file_header->ar_name,  *file_mode);

			
		struct stat *f_stat = malloc(sizeof(struct stat));
		fstat(extf_fd, f_stat);
		

		int f_blocks = (int)f_stat->st_blocks;
		char* f_buffer[f_blocks];
		int remaining_size = filesize;
		int actual_size;

		while(remaining_size > f_blocks){
			actual_size = read(ar_fd,f_buffer,f_blocks);
			write(extf_fd,f_buffer,actual_size);
			remaining_size -= actual_size;
		}
		actual_size = read(ar_fd,f_buffer,remaining_size);
		write(extf_fd,f_buffer,actual_size);

		
		close(extf_fd);
		//change time ?
		utime(file_header->ar_name,file_utime);
		
		//lseek to the head?
		lseek(ar_fd,0,8);
	}
	close(ar_fd);
}

void ar_app(int ar_fd, int argc, char **argv){
	//DONE
	//passing in all argument as the main method because we want the list of file names
	//printf("%s\n",argv[3]);
	int file_num = argc - 3;
	int i;
	char* f_name;
	lseek(ar_fd,0,SEEK_END);	
	for (i=3;i<argc;i++){
		
		f_name = argv[i];
		//printf("%s\n",f_name);
		int target_fd;
		struct ar_hdr *f_hdr = malloc(sizeof(struct ar_hdr));
		struct stat *f_stat = malloc(sizeof(struct stat));
		target_fd = open(f_name, O_RDONLY); //the file to be appended
		
		if (target_fd == -1){
			printf("Error: Target file to append not found\n");
			close(target_fd);
			close(ar_fd);
			exit(-1);
		}
		//get metadata of the file 
		fstat(target_fd, f_stat);
		int f_blksz = (int)f_stat->st_blksize;

		char namestr[strlen(f_name)+1];
		strcpy (namestr,f_name);		
		strcat(namestr, "/");
		
		/*
		name 16
		date 12
		uid 6
		gid 6
		mode 8
		size 10
		famg 2
		
		*/
		sprintf(f_hdr->ar_name, "%-16s", namestr);
		sprintf(f_hdr->ar_date,"%-12d",(int)f_stat->st_mtime);
		sprintf(f_hdr->ar_uid,"%-6d", (int)f_stat->st_uid);
		sprintf(f_hdr->ar_gid,"%-6d", (int)f_stat->st_gid);
		
		sprintf(f_hdr->ar_mode,"%-8o",f_stat->st_mode);
		sprintf(f_hdr->ar_size,"%-10d",(int)f_stat->st_size);
		sprintf(f_hdr->ar_fmag,"%-2s",ARFMAG);	
		//printf("%s\n",f_hdr);
		write(ar_fd,f_hdr,sizeof(struct ar_hdr));
		
		//copy the content of the file
		int f_blocks = (int)f_stat->st_blocks;
		char* f_buffer[f_blocks];
		int actual_size;
		while(actual_size = read(target_fd,f_buffer,f_blocks)){
			
			write(ar_fd,f_buffer,actual_size);

		}
		
		if ((lseek(ar_fd,0,SEEK_END)%2)){
			write(ar_fd,"\n",1);
		}
		
		close(target_fd);

	}
	close(ar_fd);
	exit(0);

}

void ar_appall(int ar_fd, int argc, char **argv){
	//TODO- DONE but need to check
	//artificially generate a list as argv and argc then pass into ar_app :)
	//check current dir, add all file mod in 1h except argv[0] to new_argv
	int count = 0;
	
	DIR *dp;
	dp = opendir("./");
	struct dirent *ep;
	char* new_argv[10000];
	new_argv[0] = argv[0];
	new_argv[1] = "q";
	new_argv[2] = argv[2];
	int new_argc = 2;
	
	while (ep = readdir(dp) ){
		if ((ep->d_type == DT_REG) && (strcmp(ep->d_name, argv[2]) != 0)){
			char* filename = ep->d_name;
			struct stat *f_stat = malloc(sizeof(struct stat));
			stat(filename,f_stat);
			//printf("%s\n",filename);
			//printf("%f\n",difftime(time(NULL), f_stat->st_mtime));
			if  ( difftime(time(NULL), f_stat->st_mtime)<=3600){
				//printf("%s\n",filename);
				new_argc ++;
				new_argv[new_argc] = filename;
				

			}
		}
		

	}
	ar_app(ar_fd,new_argc+1,new_argv);

	

}

void ar_del(int ar_fd, int argc, char **argv){
	//TODO
	//read through the ar file
	//maintain a list of file 2b deleted
	//if (inlist){skip;inlist.rm()file;} else {read and write}	

	char* to_del[argc - 3];
	int num_to_del = argc - 3;
	int i;
	struct stat *f_stat = malloc(sizeof(struct stat));
	for (i=0; i<argc-3;i++){
		to_del[i] = argv[i+3];
	}
	//creat a new file
	unlink(argv[2]);
	int new_ar_fd= open(argv[2],O_RDWR|O_CREAT,0666);
	write(new_ar_fd, ARMAG, 8);
	fstat(new_ar_fd, f_stat);
	int f_blocks = (int)f_stat->st_blocks;

	//each time read a header and go thru the del_list
	struct ar_hdr * file_header = malloc(sizeof(struct ar_hdr));

	while(read(ar_fd, file_header, sizeof(struct ar_hdr)) == sizeof(struct ar_hdr)){
		//if inlist?
		
		char *filename = malloc(20);
		sprintf(filename, "%-16s", file_header->ar_name);		
		int j;
		
		for (j=16;j>=0;j--){
			if (filename[j] == '/'){
				filename[j] = '\0';
				break;
			}

		}
		//printf("Searching %s\n",filename);
		int flag = 0;
		for (j=0;j<argc-3;j++){
			//printf("against %s\n",to_del[j]);			
			if ((strcmp(filename, to_del[j]) == 0)){
				
				flag = 1;			
				to_del[j] = "\n";
				break;

			}
		}
		
		int filesize = (int)strtol(file_header->ar_size,NULL,10);
		if (flag == 0){
			//write



			write(new_ar_fd,file_header,sizeof(struct ar_hdr));
			
			//copy the content of the file

			char* f_buffer[f_blocks];
			int remaining_size = filesize;
			int actual_size;

			while(remaining_size > f_blocks){
				actual_size = read(ar_fd,f_buffer,f_blocks);
				write(new_ar_fd,f_buffer,actual_size);
				remaining_size -= actual_size;
			}
			actual_size = read(ar_fd,f_buffer,remaining_size);
			write(new_ar_fd,f_buffer,actual_size);
		
			if ((lseek(new_ar_fd,0,SEEK_CUR)%2)){
				write(new_ar_fd,"\n",1);
			}

			if ((lseek(ar_fd,0,SEEK_CUR)%2)){
				lseek(ar_fd,1,SEEK_CUR);
			}

		} else {
			//skip
			
			if (filesize%2) filesize++;
			lseek(ar_fd,filesize,SEEK_CUR);
			printf("going thru:%s\n",file_header->ar_name);
			
		}

	}

	close(ar_fd);
	close(new_ar_fd);
	exit(0);
}

