#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<unistd.h>
#include<stdlib.h>
#include<fcntl.h>
#define MAPLEN 0x1000
struct STU
{
	int id;
	char name[20];
	char sex;
};
void sys_err(char *str,int exitno)
{
	perror(str);
	exit(exitno);
}
int main(int argc,char *argv[])
{
	struct STU *mm;
	printf("ddd");
	int fd;
	if(argc<2)
	{
		printf("./a.out filename \n");
		exit(1);
	}
	fd = open(argv[1],O_RDWR | O_CREAT,0777);// open a file /create a new file
	if(fd<0)
		sys_err("open",1);
	if(lseek(fd,MAPLEN-1,SEEK_SET)<0)
		sys_err("lseek",3);//offset wen jian pianyi
	if(write(fd,"\0",1)<0)
		sys_err("write",4);
	mm = mmap(NULL,MAPLEN,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);//read/write
	
	if(mm==MAP_FAILED)
		sys_err("mmap",2);
	close(fd);
	int i=0;
	while(1)
	{
		mm->id = i;
		sprintf(mm->name,"zhang-%d",i);
		if(i%2==0)
			mm->sex = 'm';
		else
			mm->sex = 'f';
		i++;
		sleep(1);
	}
	munmap(mm,MAPLEN);//release neicun
	return 0;
}