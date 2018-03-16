#include<stdio.h>
#include<string.h>
#include <stdlib.h>


int charToInt(char c);

int main(int argc, char *argv[]) {

	FILE *file;
	int l, n, lineLength;
	int** table;


	file = fopen(argv[1], "r");
	

	if (file) {
		l = charToInt(getc(file));
		n = l * l;
		lineLength = l + (l - 1);

		table = (int **)malloc(n * sizeof(int*));
		for(int i = 0; i < l; i++){
			table[i] = (int *)malloc(n * sizeof(int));	
		} 

	    fclose(file);
	}
	printf("%d",l);
}

int charToInt(char c){ return c - '0';}