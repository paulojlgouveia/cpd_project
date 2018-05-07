
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
machine=9
# owned directory inside the cluster
start_dir='paulo/'
# source file to compile
src_file='sudoku-mpi.c'
# arguments for the executable
args='input/9x9.in'



install:
	sudo apt-get install expect
	
	
permissions:
	chmod +x .compile.sh
	chmod +x .login-cluster.sh
	chmod +x .login-rnl.sh
	chmod +x .open-output.sh
	chmod +x .run.sh
	chmod +x .upload-files.sh


	
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

	