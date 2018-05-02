#!/usr/bin/expect -f

set timeout [lindex $argv 0]
set user [lindex $argv 1]
set rnl_pw [lindex $argv 2]
set machine [lindex $argv 3]
set cluster_pw [lindex $argv 4]
set start_dir [lindex $argv 5]
set args [lindex $argv 6]




spawn ssh $user@cluster.rnl.tecnico.ulisboa.pt
expect "*?assword:*"
send -- $rnl_pw\r

expect "$user@borg:*"
send -- "ssh cpd04@cpd-$machine\r"
expect "*?assword:*"
send -- $cluster_pw\r

expect "Have a lot of fun..."
send -- "cd $start_dir; clear\r"
expect "*cpd04@*"
send -- "mpirun --hostfile nodes.txt exe $args > output.txt\r"


while {1} {
	sleep 1
	expect {
		eof							{ break }
		"The authenticity of host"	{ send "yes\r" }
		"*cpd04@*"					{ send "exit\r" }
	}
}


# sleep 1
wait
expect "$user@borg:*"
send -- "exit\r"

close $spawn_id


