#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>  // CONSTANTS for open sys call


int main(void)
	{  	int fd;  
		int bytes=801743*8;
		char *buf=new char[bytes];  
		int ret;
        int i;
		i=0;
	fd=open("/build/ilija/user.ilijav.HCtest.1/random.bin",O_RDONLY,0);
	if(fd==-1) { perror(NULL);  return 1; }

	// buffer is better
	while((ret=read(fd,buf,sizeof(buf) ) )>0)
		{  i=i+1;
	//	for(i=0;i<ret;++i)
	//		{  printf("%c",buf[i] );  }
		}
       printf("%d",i);
	close(fd);  return 0;
	}
