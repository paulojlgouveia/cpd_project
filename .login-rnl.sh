#!/usr/bin/expect -f

set timeout [lindex $argv 0]
set user [lindex $argv 1]
set rnl_pw [lindex $argv 2]


spawn ssh $user@cluster.rnl.tecnico.ulisboa.pt
expect "*?assword:*"
send -- $rnl_pw\r

interact

