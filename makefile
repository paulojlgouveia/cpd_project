
# increase if needed due to network latency
timeout=10
# ist username and password
user='ist175657'
pw='L18fPGfPjlU'
# directory inside afs to temporarily store files
afs_dir='~/'
# group password for the cluster
cluster_pw='QBtMUdds7PAx0dDoZQ/c'
# cluster server (cpd04@cpd-$machine)
machine=6
# owned directory inside the cluster
start_dir='paulo/'
# source file to compile
src_file='sudoku-mpi.c'
# arguments for the executable
args='input/9x9.in'


SERIAL_FLAGS=-fopenmp -ansi -pedantic -Wall -Wno-unused-result -O3 -lm -std=c99
MPI_FLAGS=-fopenmp -ansi -pedantic -Wall -Wno-unused-result -O3 -lm -std=c99


install:
	sudo apt-get install expect
	sudo apt-get install libopenmpi-dev openmpi-bin

permissions:
	chmod +x .compile.sh
	chmod +x .login-cluster.sh
	chmod +x .login-rnl.sh
	chmod +x .open-output.sh
	chmod +x .run.sh
	chmod +x .upload-files.sh


#################################################################################################

clean:
# 	@clear
	@rm -f *.o
	@rm -f sudoku-serial sudoku-omp sudoku-mpi
	@rm -f output/*
	@rm -f *.zip
	@rm -f massif.out.*


serial:
	gcc $(SERIAL_FLAGS) sudoku-serial.c -o sudoku-serial
	valgrind -v --leak-check=full --show-leak-kinds=all ./sudoku-serial input/9x9.in


mpi:
	mpicc $(MPI_FLAGS) sudoku-mpi.c -o sudoku-mpi
	mpirun -np 6 sudoku-mpi input/4x4.in
	mpirun -np 6 sudoku-mpi input/4x4-nosol.in
	mpirun -np 6 sudoku-mpi input/9x9.in
	mpirun -np 8 sudoku-mpi input/9x9-nosol.in
# 	mpirun -np 8 sudoku-mpi input/16x16.in
# 	valgrind -v --leak-check=full --show-leak-kinds=all mpirun -np 4 sudoku-mpi input/9x9.in
	
	
	
#################################################################################################
	
login-rnl:
	./.login-rnl.sh ${timeout} ${user} ${pw}

login-cluster:
	./.login-cluster.sh ${timeout} ${user} ${pw} ${cluster_pw} ${machine} ${start_dir}

	
upload-files:
	./.upload-files.sh ${timeout} ${user} ${pw} ${machine} ${cluster_pw} ${start_dir}

	
compile:
	./.compile.sh ${timeout} ${user} ${pw} ${machine} ${cluster_pw} ${start_dir} ${src_file}

run:
	./.run.sh ${timeout} ${user} ${pw} ${machine} ${cluster_pw} ${start_dir} ${args}

compile-run: compile run


show:
	./.open-output.sh ${timeout} ${user} ${pw} ${cluster_pw} ${machine} ${start_dir}



	
	