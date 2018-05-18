
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


FILES=4x4.in 4x4-nosol.in 9x9.in 9x9-nosol.in 16x16-zeros.in 16x16.in 16x16-nosol.in
TURNS=0 1 2 3 4 5 6 7 8 9
PROCESSES=2 4 8 16


#################################################################################################

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

	
compile-serial:
	gcc $(SERIAL_FLAGS) sudoku-serial.c -o sudoku-serial
	
compile-mpi:
	mpicc $(MPI_FLAGS) sudoku-mpi.c -o sudoku-mpi
	

serial: compile-serial
	@for f in ${FILES}; do \
		echo '\n'$$f; \
		(for i in ${turns}; do \
			./sudoku-serial input/$$f; \
		done) \
	done


mpi: compile-mpi
	@for f in ${FILES}; do \
		for p in ${PROCESSES}; do \
			echo '\n'$$p' - '$$f; \
			for i in ${TURNS}; do \
				mpirun -np $$p sudoku-mpi input/$$f; \
			done \
		done \
	done
	
	
	
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



	
	