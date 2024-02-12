#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {

	char * word = (char *) malloc(16);
	while(fgets(word, 16, stdin) != NULL) {
		char * first_num = strtok(word, " ");
		char * second_num = strtok(NULL, " ");

		int x = atoi(first_num);
		int y = atoi(second_num);

		printf("x: %d, y: %d\n", x, y);
	}

	return 0;

}
