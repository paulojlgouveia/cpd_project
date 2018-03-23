	
/************************************************************************
 * 																		*
 * Parallel and Distributed Computing 									*
 * project part 1														*
 * 																		*
 * group 4, Alameda														*
 * 																		*
 * 75657 Paulo Gouveia													*
 * 76213 Gonçalo Lopes													*
 * ##### ## ##															*
 * 																		*
 * Serial implementation												*
 * 																		*
 ************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 8 spaces, 9 cells, '\0', '\n', possible '\r' on windows
unsigned int MAX_LINE_SIZE = 20; 

typedef struct {
	int *candidates;
	int x, y;
	int checked;
} position;

typedef struct{
	int x,y;
} coordinate;



int* arrayNAND(int* a1, int* a2);

void printCell(position* p, int size);
int charToInt(char c);
void printBoard(position **board, int N);

coordinate getBlockCoordinate(position *cellPosition, int l);
position *getBlockTopLeft(position*** board, coordinate blockCoordinate, int l);

int solved(position **board, int N);
void checkBlockLineColumn(position*** board,int l,int n, position *pos) ;

int* arrayNAND(int* a1, int* a2);

void uniqueCanditate();
void removeCandidates(int* a1, int* a2, int n);

void Position(position *p, int size, int x, int y);

int main(int argc, char *argv[]) {
	
	FILE *file;
	int l, n;
	int value;
	position** board;
	char line[MAX_LINE_SIZE];
	char* cell;
	
	if(argc < 2){
		printf("missing argument.\nusage: sudoku-serial <filename>\n");
		exit(1);
	}
	
	// read initial board from the input file
	file = fopen(argv[1], "r");
	if (file == NULL) {	
		printf("failed to read input file: '%s'\n", argv[1]);
		exit(2);
	 
	} else {
		// read first line (board size)
		fgets(line, MAX_LINE_SIZE, file);
		sscanf(line,"%d", &l);
		n = l * l;
// 		printf("l = %d, n = %d\n", l, n);
		
		board = (position**) malloc(n * sizeof(position*));
		for (int i = 0; i < n; i++) {
			board[i] = (position*) malloc(n * sizeof(position));
			for (int j = 0; j < n; j++) {
				Position(&board[i][j], n, i, j);
			}
		}
		
		
		// initialize the board
		for (int i = 0; i < n ; i++) {
			if (fgets(line, MAX_LINE_SIZE, file) != NULL) {
				
				cell = strtok(line, " ");
				for (int j = 0; j < n; j++) {
					value = charToInt(*cell);
						
					if (value > 0)
						board[i][j].checked = value;
						
					else
						for (int t = 1; t <= n; t++)
							board[i][j].candidates[t-1] = t;
					
					cell = strtok(NULL, " ");
				}
				
			} else {
				printf("ill formed file '%s'\n", argv[1]);
				exit(3);
			}
		}
		
	    fclose(file);
	}
	
	
	// <FIXME> solve the thing here
	
	
	
	// </FIXME>
	
	
	

	// display solution
	printBoard(board, n);
// 	char* s = (solved(board, n) != 0)? "True " : "False";
// 	printf("%s\n", s);
	
	// free the memory
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			free(board[i][j].candidates);
		}
		free(board[i]);
	}     
	free(board);
	
	return 0;
}

void Position(position *p, int size, int x, int y) {
// 	position *p = malloc(sizeof(position));
	p->candidates = malloc(size * sizeof(int));
	p->x = x;
	p->y = y;
	p->checked = 0;
}

void printBoard(position **board, int N) {
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			printf("%d ", board[i][j].checked);
		}
		printf("\n");
	}
// 	printf("\n");
}

void printCell(position* p, int size) {
// 	char* checked = (p->checked == 0)? "True " : "False";
// 	printf("(%d,%d) %s [", p->x, p->y, checked);
	
	printf("(%d,%d) %d [", p->x, p->y, p->checked);
	for (int t = 0; t < size; t++)
		printf(" %d", p->candidates[t]);
	
	printf(" ]\n");
}

int charToInt(char c) {
	return c - '0';
}

//Returns a 0-indexed coordinate of the block
//e.g., in a 9x9, 3x3 blocks -> (5,5) returns (1,1), (8,8) returns (2,2)
coordinate getBlockCoordinate(position *cellPosition, int l){
	coordinate blockCoordinate;

	blockCoordinate.x = cellPosition->x / l;
	blockCoordinate.y = cellPosition->y / l;

	return blockCoordinate;
}

//returns the block's top left position
//e.g., in a 4x4 sudoku, 2x2 blocks -> (1,0) returns (2,0)
position *getBlockTopLeft(position*** board, coordinate blockCoordinate, int l){ 
	return board[blockCoordinate.x * l][blockCoordinate.y * l]; 
}

int solved(position **board, int N) {
	for (int i = 0; i < N; i++)
		for (int j = 0; j < N; j++)
			if (board[i][j].checked == 0)
				return 0;
			
	return 1;
}


void checkBlockLineColumn(position*** board,int l,int n, position *pos) {
	int i,numX, numY, numBlock, removables[n];
	position *blockPos = getBlockTopLeft(board,getBlockCoordinate(pos,l),l);

	//Gather candidates to remove in row and column
	for(i = 0; i < n;i++){
		
		numX = board[i][pos->y]->checked;
		numY = board[pos->x][i]->checked;
		numBlock = board[blockPos->x + (i % n)][blockPos->y + ((int) (i/n))]->checked;
		// check in row
		if(numX)
			removables[numX] = numX;
		
		// check in column
		if(numY)
			removables[numY] = numY;
		
		// check in block
		if(numBlock)
			removables[numBlock] = numBlock;
	}
	
	removeCandidates(pos->candidates, removables, n);
}

void removeCandidates(int* a1, int* a2, int n){
	for(int i = 0; i < n; i++){
		if(a2[i])
			a1[i] = i+1;
	}
}

void uniqueCanditate(position *pos, int n) {
	int count = 0, unique;
	for(int i  = 0; i < n; i++){
		unique = pos->candidates[i];
		if(unique)
			count++;
	}
	if(count == 1)
		pos->checked = unique;
}
//****************Might not be needed**********************
void lineRemoval() {
	// FIXME
	
	return;
}


void blockRemoval() {
	// FIXME
	
	return;
}


void nakedSubset() {
	// FIXME
	
	return;
}


void hiddenSubset() {
	// FIXME
	
	return;
}


void xWing() {
	// FIXME
	
	return;
}


void swordfish() {
	// FIXME
	
	return;
}
