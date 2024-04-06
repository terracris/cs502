#include <stdio.h>
#include <stdlib.h>

int main() {

    const char * test = "1200";
    int num = atoi(test);

    printf("%d\n", (num * 2));
    return 0;
}