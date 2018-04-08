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

typedef struct simple{
	int x, y;
} simple;

typedef struct element {
	int x, y;
	int value;
	int expanded;
	int **table;
	struct element* next;
} element;

element* head = NULL;
element* tail = NULL;
int** solution = NULL;
int N, L, size = 0;

/****************************************************************************/

puzzle* Puzzle(int modSize);
element* Element(int** table, int x, int y, int value);
simple Simple(int x, int y);
puzzle* getPuzzleFromFile(char *inputFile);
void freePuzzle(puzzle *board);
void freeStack();

void printSolution(int** table);
void printBoard(int** table);
void printStack();

int isValid(int** table, int row, int column, int number);
int solved(int** table);

void initializeStack(int** table);
void pushElement(element* el);
element* popElement();
simple getNextElement(int** table, int x, int y);
void genNodes(element* parent);

int iterativeSolve(puzzle* board, int nthreads);

/****************************************************************************/

int main(int argc, char *argv[]) {

	puzzle *board;
	double time;
	if(argc < 3){
		printf("missing argument.\nusage: sudoku-serial <filename> <thread_count>\n");
		exit(1);
	}
	
	board = getPuzzleFromFile(argv[1]);
	
	omp_set_dynamic(0);
	omp_set_num_threads(atoi(argv[2]));
	solution = (int**) malloc(N * sizeof(int*));
	for (int i = 0; i < board->N; i++) 
		solution[i] = (int*) malloc(N * sizeof(int));

	time = omp_get_wtime();
	if (iterativeSolve(board, atoi(argv[2]))) {
		printBoard(solution);
		
	} else {
		printf("No solution.\n");
	}
	time = omp_get_wtime() -time;
	
	printf("Time: %f seconds\n",time);
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

element* Element(int** table, int x, int y, int value) {
	element* el = (element*) malloc(sizeof(element));

	el->x = x;
	el->y = y;
	el->value = value;
	el->expanded = 0;
	
	el->table = (int**) malloc(N * sizeof(int*));
	for (int i = 0; i < N; i++) {
		el->table[i] = (int*) malloc(N * sizeof(int));
		for(int j = 0; j < N;j++){
			el->table[i][j] = table[i][j];
		}
	}

	el->table[x][y] = value;
	el->next = NULL;
	return el;
}

simple Simple(int x, int y) {
	simple s;
	s.x = x;
	s.y = y;

	return s;
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
		N = modSize * modSize;
		L = modSize;
		
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
	free(board->table);
	free(board);
}

void freeElement(element* el) {
	for (int i = 0; i < N; i++) {
		free(el->table[i]);
	}  
	free(el->table);
	free(el);
}

void freeStack(){
	element* curr = head, *aux;
	printStack();
	while(curr != NULL){
		//printf("curr %d on (%d,%d)\n",curr->value,curr->x,curr->y);
		aux = curr;
		curr = curr->next;
		freeElement(aux);

	}
}



/****************************************************************************/

void printSolution(int** table) {
	for (int i = 0; i < N; i++) {
		for (int j = 0; j < N; j++) {
			printf("%d ", table[i][j]);
		}
		printf("\n");
	}
}

void printBoard(int** table) {
	
	if (N > 9) {
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				printf("%2d ", table[i][j]);
			}
			printf("\n");
		}
		
	} else {
		for (int i = 0; i < N; i++) {
			for (int j = 0; j < N; j++) {
				printf("%d ", table[i][j]);
			}
			printf("\n");
		}
	}
	printf("\n");
}

void printStack() {
	element* curr = head;
	int t = 0;
	printf("stack start\n");
	while(curr != NULL){
		printf("%4d: (%d,%d) %2d\n", t, curr->x, curr->y, curr->value);
		curr = curr->next;
		t++;
	}
	
	printf("stack end\n");
}


/****************************************************************************/


int isValid(int** table, int x, int y, int number) {
		
    int rowStart = (x/L) * L;
    int colStart = (y/L) * L;
    for(int i = 0; i < N; ++i) {
        if (table[x][i] == number)
			return 0;
		
        if (table[i][y] == number)
			return 0;
		
		int iFromBlock = rowStart + (i % L);
		int jFromBlock = colStart + (i / L);
		
        if (table[iFromBlock][jFromBlock] == number)
			return 0;
    }
    
    return 1;
}

int solved(int** table) {
	for (int i = N-1; i > -1; i--)
		for (int j = N-1; j > -1; j--)
			if (table[i][j] == 0)
				return 0;
		
	return 1;
}


/****************************************************************************/

void initializeStack(int** table) {
	int value, valid;
	simple firstPos;
	element* child;

	firstPos = Simple(-1,-1);
	for(int i = 0; i < N; i++){
		for(int j = 0; j < N; j++){
			if(!table[i][j]){
				//printf("first empty: (%d,%d)\n",i, j);
				firstPos = Simple(i, j);
				i = j = N;
				break;
			}
		}
	}
	if(firstPos.x == -1){
		return;
	}

	for(value = 1; value <= N; value++){
		valid = isValid(table, firstPos.x, firstPos.y, value);
		//printf("%d is valid in (%d,%d)? -> %d\n",value,next->x,next->y,valid);
		if(valid){
			child = Element(table, firstPos.x,firstPos.y,value);
			//printf("inserting %d from (%d,%d)\n",value,meganext->x,meganext->y);
			#pragma omp critical
			{
				pushElement(child);				
			}
			//printf("inserted\n");
			//printStack();
		}
	}
}

void pushElement(element* el) {
	if(head == NULL){
		head = el;
		tail = el;
	}
	else{
		el->next = head;
		head = el;
	}
	size++;
}

element* popElement() {
	element* aux = NULL;
	if(head != NULL){
		aux = head;
		head = head->next;
		if(head == NULL)
			tail = NULL;
	}
	size--;
	return aux;
}

simple getNextElement(int** table, int x, int y) {
	int i, j;
	simple next = Simple(-1,-1);

	if(y == N -1 && x == N -1)
		return next;
	else if (y == N -1 && x < N -1){
		i = x  + 1;
		j = 0;
	}
	else{
		i = x;
		j = y + 1;
	}
	for(;i < N; i++){
		for(; j < N; j++){
			//printf("isempty?: (%d,%d)\n",i, j);
			if(!table[i][j]){
				//printf("first empty: (%d,%d)\n",i, j);
				next = Simple(i, j);
				i = j = N;
				break;
			}
		}
		j= 0;
	}
	//printf("previous: (%d,%d)\n\n",x,y);
	//printf("next: (%d,%d)\n",next->x,next->y);
	return next;
}

void genNodes(element* parent) {
	int value,valid;
	int** table;
	int x, y;

	simple nextPos;
	element* child;

	if(parent == NULL){
		return;
	}

	x = parent->x;
	y = parent->y;
	table=parent->table;
	nextPos = getNextElement(table,x,y);

	if(nextPos.x == -1){
		return;
	}

	for(value = 1; value <= N; value++){
		valid = isValid(table, nextPos.x, nextPos.y, value);
		//printf("%d is valid in (%d,%d)? -> %d\n",value,nextPos.x,nextPos.y,valid);
		if(valid){
			child = Element(table, nextPos.x,nextPos.y,value);
			//printf("inserting %d from (%d,%d)\n",value,nextPos.x,nextPos.y);
			#pragma omp critical
			{
				pushElement(child);				
			}
			//printf("inserted\n");
			//printStack();
		}
	}
	//printf("im gonna free %d from (%d,%d)\n",next->value,next->x,next->y);
	//printStack();
	//printStack();
	//printf("what?\n");
}

/****************************************************************************/

int iterativeSolve(puzzle* board, int nthreads) {
	int hasSolution = 0, terminated = 0, sumStucks = 0;
	element* top;

	int stucks[nthreads];
	element* current = NULL;

	/*Initialize nthreads boards, one for each thread to work on*/
	for(int t=0;t<nthreads;t++){
 		stucks[t] = 0;
	}

	/*Initialize the stack with the first node*/
	initializeStack(board->table);
	
	//printStack();
	#pragma omp parallel default(shared) private(current,top) firstprivate(sumStucks)
	{
		int tid = omp_get_thread_num();

		while(!terminated){
			//printf("size=%d\n",size);
			//printf("\n****Thread(%d) vvv****\n",tid);

			//printStack();
			#pragma omp critical
			{
				//printf("\n--Thread(%d) popping!!\n",tid);
				current = popElement();
				/*if(current != NULL)
					//printf("*****sCurrent(%d): %d in (%d,%d)\n", tid ,current->value,current->x,current->y);
				else{
					//printf("Current(%d): NULL :(\n", tid);
				}*/
			}
			
			//printf("?");
			//getchar();
			if(current != NULL){
				//printf("Current(%d): %d in (%d,%d)\n", tid, current->value,current->x,current->y);
			//	printBoard(current->table);
				genNodes(current);
				
				//printf("stillAlive\n");
				if(solved(current->table)){
					//printf("finishing!(%d)\n",tid);
					terminated = 1;
					hasSolution = 1;
					for(int i= 0;i < N;i++)
						for(int j = 0;j < N; j++)
							solution[i][j] = current->table[i][j];
				}
				//printf("YESYES\n");
				freeElement(current);
				//printf("NONO\n");
			}
			else{
				//printf("\n--Thread(%d) is here!!\n",tid);
				top = head;				
				while(top == NULL && sumStucks < nthreads && !terminated){
					//printf("i'm stuck hhere (%d)\n",tid);
					stucks[tid] = 1;
					sumStucks = 0;
					for(int t = 0; t < nthreads; t++) {
						sumStucks += stucks[t];
					}
					top = head;				
					//printf("???\n");
					#pragma omp flush

				}
				for(int t = 0; t < nthreads; t++) {
					stucks[t] = 0;
				}
				if(sumStucks == nthreads){
					//printf("ITS OVER! sum =%d\n",sumStucks);
					hasSolution = 0;
					terminated = 1;
				}
				//if(terminated)
					//printf("BAILED!(%d)\n",tid);
			}
		}
	}
	/*printf("IS IT HERE?\n");
	freeStack();
	printf("NO\n");*/
	return hasSolution;
}

// 	element stack[board->N * board->N * board->N];
	/*element stack[MAX_STACK_SIZE];
	int stackPtr = -1;
	int progress = 0;
	#pragma omp parallel for
	for (int i = 0; i < board->N; i++) {
		for (int j = 0; j < board->N; j++) {
			
			// if empty cell
			if (!board->table[i][j]) {
				for (int value = board->N; value > 0; value--) {
					// add candidates to stack
					if (isValid(board, i, j, value)) {
						stackPtr++;
						stack[stackPtr].x = i;
						stack[stackPtr].y = j;
						stack[stackPtr].value = value;
						stack[stackPtr].expanded = 0;
						
						progress = 1;
					}
				}
				
				// if no candidates added, revert last branch of changes
				if (!progress) {
					while (stack[stackPtr].expanded) {
						i = stack[stackPtr].x;
						j = stack[stackPtr].y;
						board->table[i][j] = 0;
						stackPtr--;
					}
				}
				
				if (stackPtr >= 0) {
					// pick a candidate for the next iteration
					i = stack[stackPtr].x;
					j = stack[stackPtr].y;
					board->table[i][j] = stack[stackPtr].value;
					stack[stackPtr].expanded = 1;
					
					progress = 0;
					
				} else {
					// nothing left to try, there is no solution
				}
			}
		}
	}
	
	return solved(board);*/

/*
int isValid(puzzle* board, int row, int column, int number) {
		
    int rowStart = (row/board->L) * board->L;
    int colStart = (column/board->L) * board->L;
    int flag = 1;    	
    int iFromBlock;
    int jFromBlock;

    #pragma omp parallel
    for(int i = 0; i < board->N; ++i) {
        
        if (board->table[row][i] == number) {
            #pragma omp critical
            {
                flag = 0;
            }
            #pragma omp cancel parallel
        }
        
	if (board->table[i][column] == number) {
            #pragma omp critical
            {
                flag = 0;
            }
            #pragma omp cancel parallel
        }

        iFromBlock = rowStart + (i % board->L);
        jFromBlock = colStart + (i / board->L);
        
	if (board->table[iFromBlock][jFromBlock] == number) {
            #pragma omp critical
            {
                flag = 0;
            }
            #pragma omp cancel parallel
        }
    }
    return flag;
}*/