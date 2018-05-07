#!/usr/bin/expect -f

set timeout [lindex $argv 0]
set user [lindex $argv 1]
set rnl_pw [lindex $argv 2]
set machine [lindex $argv 3]
set cluster_pw [lindex $argv 4]
set start_dir [lindex $argv 5]



spawn sftp $user@cluster.rnl.tecnico.ulisboa.pt
expect "*?assword:*"
send -- $rnl_pw\r

expect "sftp>"
send -- "put -r input\r"

expect "sftp>"
send -- "put nodes.txt\r"

expect "sftp>"
send -- "exit\r"


spawn ssh $user@cluster.rnl.tecnico.ulisboa.pt
expect "*?assword:*"
send -- $rnl_pw\r

expect "*@borg:*"
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
send -- "put -r input\r"

expect "sftp>"
send -- "put nodes.txt\r"

expect "sftp>"
send -- "exit\r"


sleep 1
expect "*@borg:*"
send -- "rm -r input\r"

sleep 1
expect "*@borg:*"
send -- "rm nodes.txt\r"

expect "*@borg:*"
send -- "exit\r"

