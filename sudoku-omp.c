
/****************************************************************************
* 																			*
* Parallel and Distributed Computing 										*
* project part 1															*
* 																			*
* group 4, Alameda															*
* 																			*
* 75657 Paulo Gouveia														*
* 76213 Gonçalo Lopes														*
* 70969 Mario Reis															*
* 																			*
* OpenMP implementation														*
* 																			*
*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>

// 81 spaces, 162 cells (2 digits), '\n', '\0', possible '\r'
#define MAX_LINE_SIZE 246
// 81^2 + (81*80)/2 , board size + candidates per cell
#define MAX_STACK_SIZE 9801


/****************************************************************************/

typedef struct puzzle {
	int L, N;
	int **table;
} puzzle;

typedef struct element {
	struct element* parent;
	int x, y;
	int value;
	int expanded, openChilds;
} element;
// typedef struct Element element;

/****************************************************************************/

element* newElement(element* parent, int x, int y, int value) {
	element* new = malloc(sizeof(element));
	new->parent = parent;
	new->x = x;
	new->y = y;
	new->value = value;
	new->expanded = 0;
	new->openChilds = 0;
	
// 	if (parent != NULL)
// 		parent->openChilds++;
	
	return new;
}



/****************************************************************************/

puzzle* Puzzle(int modSize);
puzzle* getPuzzleFromFile(char *inputFile);
void freePuzzle(puzzle *board);

void printSolution(puzzle *board);
void printBoard(puzzle *board);
void printStack(element* stack, int from, int to);

int solved(puzzle* board);

int iterativeSolve(puzzle* board, int threadsN);


/****************************************************************************/

int main(int argc, char *argv[]) {

	puzzle *board;
	
	if(argc < 3){
		printf("missing argument.\nusage: sudoku-serial <filename> <thread_count>\n");
		exit(1);
	}
	
	board = getPuzzleFromFile(argv[1]);
	
	omp_set_dynamic(0);
	omp_set_num_threads(atoi(argv[2]));
	
	if (iterativeSolve(board, atoi(argv[2]))) {
		printBoard(board);
		
	} else {
		printf("No solution.\n");
	}
	
	
	freePuzzle(board);
	return 0;
}


/****************************************************************************/

puzzle* Puzzle(int modSize) {
	puzzle* board = malloc(sizeof(puzzle));
	
	board->L = modSize;
	board->N = modSize * modSize;
	
	board->table = (int**) malloc(board->N * sizeof(int*));
	for (int i = 0; i < board->N; i++) 
		board->table[i] = (int*) malloc(board->N * sizeof(int));
	
	return board;
}

puzzle* getPuzzleFromFile(char *inputFile) {
	
	FILE *file;
	int modSize;
	char line[MAX_LINE_SIZE];
	char* inputCell;
	
	puzzle* board;
	
	// read initial board from the input file
	file = fopen(inputFile, "r");
	if (file == NULL) {	
		printf("failed to read input file: '%s'\n", inputFile);
		exit(2);
	
	} else {
		// read first line (board size)
		fgets(line, MAX_LINE_SIZE, file);
		sscanf(line,"%d", &modSize);
		
		// initialize the board
		board = Puzzle(modSize);
		
		for (int i = 0; i < board->N; i++) {
			if (fgets(line, MAX_LINE_SIZE, file) != NULL) {
				inputCell = strtok(line, " ");
				for (int j = 0; j < board->N; j++) {
					board->table[i][j] = atoi(inputCell);					
					inputCell = strtok(NULL, " ");
				}
				
			} else {
				printf("ill formed file '%s'\n", inputFile);
				exit(3);
			}
		}
		
		fclose(file);
	}
	
	return board;
}

void freePuzzle(puzzle *board) {
	for (int i = 0; i < board->N; i++) {
		free(board->table[i]);
	}  
	
	free(board);
}


/****************************************************************************/

void printSolution(puzzle *board) {
	for (int i = 0; i < board->N; i++) {
		for (int j = 0; j < board->N; j++) {
			printf("%d ", board->table[i][j]);
		}
		printf("\n");
	}
}

void printBoard(puzzle *board) {
	
	if (board->N > 9) {
		for (int i = 0; i < board->N; i++) {
			for (int j = 0; j < board->N; j++) {
				printf("%2d ", board->table[i][j]);
			}
			printf("\n");
		}
		
	} else {
		for (int i = 0; i < board->N; i++) {
			for (int j = 0; j < board->N; j++) {
				printf("%d ", board->table[i][j]);
			}
			printf("\n");
		}
	}
	printf("\n");
}

void printStack(element* stack, int from, int toInclusive) {
	for (int t = from; t <= toInclusive; t++)
		printf("%4d: (%d,%d) %2d\n", t, stack[t].x, stack[t].y, stack[t].value);
	
	printf("\n");
}


/****************************************************************************/

int solved(puzzle* board) {
	for (int i = board->N-1; i > -1; i--)
		for (int j = board->N-1; j > -1; j--)
			if (board->table[i][j] == 0)
				return 0;
		
	return 1;
}


/****************************************************************************/

int iterativeSolve(puzzle* board, int threadsN) {
	
	omp_lock_t openStackLock;
	omp_init_lock(&openStackLock);
	
	element* openStack[MAX_STACK_SIZE];
	int stackPtr = -1;
	int solved = 0;
	
	int i = 0, j = 0, t = 0;
	int value, isValid;
	int xBlockStart, xBlock, yBlockStart, yBlock;
	
	int added = 0;
	element *queue[board->N];
	element *parent = NULL, *child = NULL;

	
	// find and expand first node
	while (i < board->N) {
		if (!board->table[i][j]) {
			added = 0;
			
			for (value = board->N; value > 0; value--) {
				isValid = 1;
				
				xBlockStart = (i/board->L) * board->L;
				yBlockStart = (j/board->L) * board->L;
				
				for(t = 0; t < board->N; t++) {
					if (board->table[i][t] == value || board->table[t][j] == value) {
						isValid = 0; break;
					}
					
					xBlock = xBlockStart + (t % board->L);
					yBlock = yBlockStart + (t / board->L);
					if (board->table[xBlock][yBlock] == value) {
						isValid = 0; break;
					}
				}
				
				if (isValid) {
					child = newElement(parent, i, j, value);
					queue[added] = child;
					added++;
				}
			}
			
			for(t = 0; t < added-1; t++) {
				stackPtr++;
				openStack[stackPtr] = queue[t];
			}
			
			break;
		}
		
		j++;
		if (j >= board->N) {
			i++; j = 0;
		}
	}
	
	// no empty cells
	if(i >= board->N)
		return 1;
	
	
	
// num_threads(threadsN) 
// #pragma omp parallel firstprivate(pathStackPtr, progress) private(i, j, t, value)
#pragma omp parallel firstprivate(i, j, t, parent, child) \
private(queue, xBlockStart, xBlock, yBlockStart, yBlock, value, isValid, added)
{
	int tid = omp_get_thread_num();
	element *test = NULL;
	
	int x, y;
	int privBoard[board->N][board->N];
	int solvedByMe = 0;
	
// 	#pragma omp for
	for (x = 0; x < board->N; x++) {
		for (y = 0; y < board->N; y++) {
			privBoard[x][y] = board->table[x][y];
		}
	}
	#pragma omp barrier
	
	
	// input was not already solved
	while(!solved) {
		solvedByMe = 1;
		
		// get unexplored node from stack
		while (parent == NULL) {
			#pragma omp critical(stackPtr)
			if (stackPtr > -1) {
				parent = openStack[stackPtr];
				stackPtr--;
				parent->openChilds--;
			}
			
			// reconstruct path
			test = parent;
			while(test != NULL) {
				privBoard[test->x][test->y] = test->value;
				test = test->parent;
			}
			
// 			printf("<%d> found empty stack, didn't even try.\n", tid);
// 			omp_set_lock(&openStackLock);
// 			while (!omp_test_lock(&openStackLock));
		}
		
		// set value
		i = parent->x;
		j = parent->y;
		privBoard[i][j] = parent->value;
		
		
// 		printf("%d (%d,%d) %d  -->%d\n", tid, i, j, privBoard[i][j], stackPtr);
		
		
		

		// advance to next free cell
		do {
			j++;
			if (j >= board->N) {
				i++; j = 0;
			}
		} while(privBoard[i][j] && i < board->N);
		
		
		// did not reach the end of the board
		if (i < board->N) {
			added = 0;
			for (value = board->N; value > 0; value--) {
				isValid = 1;
				
				xBlockStart = (i/board->L) * board->L;
				yBlockStart = (j/board->L) * board->L;
				
				for(t = 0; t < board->N; t++) {
					if (privBoard[i][t] == value || privBoard[t][j] == value) {
						isValid = 0;
						break; // skip value	
					}
					
					xBlock = xBlockStart + (t % board->L);
					yBlock = yBlockStart + (t / board->L);
					if (privBoard[xBlock][yBlock] == value) {
						isValid = 0;
						break; // skip value	
					}
				}
			
				if (isValid) {
// 					printf(">>%d>> (%d,%d)[%d] %d\n", tid, i, j, stackPtr, value);
					child = newElement(parent, i, j, value);
					queue[added] = child;
					added++;
				}
				parent->openChilds += added;
			}
			
			added--; // because added points to first free space
			#pragma omp critical(stackPtr)
			{	// add all but one child to the open nodes
				for(t = 0; t < added; t++) {
					stackPtr++;
					openStack[stackPtr] = queue[t];
				}
				
// 				omp_unset_lock(&openStackLock);
			}
			
			// keep one child for next iteration
			if (added > -1) {
				parent->expanded = 1;
				parent = queue[added];
				
			// no childs generated, backtrace changes
			} else {
				while(parent != NULL) {
					privBoard[parent->x][parent->y] = 0;
					parent = parent->parent;
				}
				
			}
			
		// end of the board, check if solved
		} else {
			for (i = board->N-1; i > -1; i--)
				for (j = board->N-1; j > -1; j--)
					if (privBoard[i][j] == 0) {
						solvedByMe = 0;
						i = j = -2;
					}
			
			if (solvedByMe) {
				printf("> %d < solved.\n", tid);
				#pragma omp critical(solved)
				solved = 1;
			}
			
		}
		
// 		omp_unset_lock(&openStackLock);
		
	}
	
	
// 	#pragma omp critical(print)
// 	{
// 		printf("> %d/%d <\n", tid, threadsN);
// 		for (i = 0; i < board->N; i++) {
// 			for (j = 0; j < board->N; j++) {
// 				printf("%2d ", privBoard[i][j]);
// 			}
// 			printf("\n");
// 		}
// 		printf("\n");
// 	}
// 	#pragma omp barrier
	
	
// 	// check all positions filled
// 	for (i = board->N-1; i > -1; i--)
// 		for (j = board->N-1; j > -1; j--)
// 			if (privBoard[i][j] == 0) {
// 				solvedByMe = 0;
// 				i = j = -2;
// 			}
	
	if (solvedByMe) {
		#pragma omp single
		{
			for (i = 0; i < board->N; i++)
				for (j = 0; j < board->N; j++)
					board->table[i][j] = privBoard[i][j];
			
// 			solved = 1;
		}
		omp_unset_lock(&openStackLock);
		#pragma omp cancel parallel
	}
}

	omp_destroy_lock(&openStackLock); 
	return solved;
}







