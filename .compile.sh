#!/usr/bin/expect -f

set timeout [lindex $argv 0]
set user [lindex $argv 1]
set rnl_pw [lindex $argv 2]
set machine [lindex $argv 3]
set cluster_pw [lindex $argv 4]
set start_dir [lindex $argv 5]
set src_file [lindex $argv 6]



spawn sftp $user@cluster.rnl.tecnico.ulisboa.pt
expect "*?assword:*"
send -- $rnl_pw\r
expect "sftp>"
send -- "put $src_file\r"
expect "sftp>"
send -- "exit\r"

close $spawn_id


spawn ssh $user@cluster.rnl.tecnico.ulisboa.pt
expect "*?assword:*"
send -- $rnl_pw\r

expect "$user@borg:*"
send -- "sftp cpd04@cpd-$machine\r"
while {1} {
	sleep 1
	expect {
		eof							{ break }
		"*?assword:*"				{ send -- $cluster_pw\r; break }
		"The authenticity of host"	{ send "yes\r" }
		"*cpd04@*"					{ send "exit\r" }
	}
}


expect "sftp>"
send -- "cd $start_dir\r"
expect "sftp>"
send -- "put $src_file\r"
expect "sftp>"
send -- "exit\r"


expect "$user@borg:*"
send -- "ssh cpd04@cpd-$machine\r"
while {1} {
	sleep 1
	expect {
		eof							{ break }
		"*?assword:*"				{ send -- $cluster_pw\r; break }
		"The authenticity of host"	{ send "yes\r" }
		"*cpd04@*"					{ send "exit\r" }
	}
}
expect "Have a lot of fun..."
send -- "cd $start_dir\r"
expect "*cpd04@*"
send -- "mpicc -fopenmp -std=c99 -g -o exe $src_file\r"



expect "*cpd04@*"
send -- "exit\r"

sleep 1
expect "$user@borg:*"
send -- "rm $src_file\r"
expect "$user@borg:*"
send -- "exit\r"

close $spawn_id

