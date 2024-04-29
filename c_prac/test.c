#include <stdio.h>
#include <unistd.h>

int main() {

  if( (fork()) < 0) {
     printf("fork error\n");
} else if (fork() == 0) {
printf("this is the child\n");
} else {
printf("this is the parent\n");
}

return 0;
}
