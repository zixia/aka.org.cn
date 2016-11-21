#!/usr/bin/perl 

print "Content-type: text/html\n\n";
open(CNT,"count.txt");

$count=<CNT> ;
$count ++;

print "<center>You are our <blink><font color=red> $count</font></blink>th visitor since Oct 1st,1998£¡</center>";
close(CNT);

open(OLD ,">count.old");
print OLD  $count;
close(OLD);
`cp  count.old count.txt`;
