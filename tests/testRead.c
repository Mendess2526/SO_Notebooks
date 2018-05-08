#include <unistd.h>
#include <fcntl.h>

int main(){
    char test[] = "teste\0lul";

    int p[2];
    pipe(p);
    if(!fork()){
        write(p[1], test, 9);
    }else{
        char c;
        close(p[1]);
        while(0 < read(p[0], &c, 1))
            write(1,&c,1);
    }
    return 0;
}
