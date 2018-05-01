#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>

ssize_t readln(int fd, void *buf, ssize_t nbyte){
	ssize_t size = 0;
	char c;
	char *buff = (char *) buf;
	while(size < nbyte && 1 == read(fd, &c, 1)){
		if(c == '\0') return size;
		buff[size++] = c;
		if(c == '\n') return size;
	}
	return size;
}

//int main(){
//	char buf[10];
//	readln(0,buf,10);
//	printf("%s",buf);
//}
