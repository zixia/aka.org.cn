#!/usr/bin/perl


$BaseDir = "../cgi-bin/votelog"; # Path to the voting section
$Counter = 0;
$Sum = 0;
$VoteNum = 0;
$NumItems = 0;
$Percentage = 0;
$Votes = 0;

$Header = <<'EOT';
<HTML>
<HEAD>
  <TITLE>投票</TITLE>
<style type="text/css">
<!--
body,table,tr,td {font-size:9pt; font-family:宋体}
a{  color: gray; text-decoration: none}
a:hover {  color: #FF0000; text-decoration: none}

td {  font-size: 9pt; font-family: "宋体"}
-->
</style>
</HEAD>
<BODY BGCOLOR="#ffffff">
<H2><CENTER><FONT SIZE=+1>AKA调查</FONT></CENTER></H2>
<hr size=1 width=60%> 
EOT

$Footer = <<'EOT';
<hr size=1 width=60%>
<center>&copy;2000 Aka Info.<BR></center>
EOT

&UnWeb;

if ($in{'option'} eq "") {
	&Error ("No Option Specified");
}

if ($in{'topic'} eq "") {
	&Error ("No Topic Specified");
}

if ($in{'option'} eq "results") {
	&DisplayResults;
}
else {
	&CheckVoter;
	&RecordVote;
}


#############################################################
# Read in the voting file and process the votes


sub CheckVoter {
	open (VF, "$BaseDir/$in{'topic'}.usr");
	$voter = <VF>;
	close VF;
	chop ($voter);
	
#	if ($voter eq $ENV{'REMOTE_HOST'}) {
#		&Error ("No Duplicate Voting");
#
#	}
	
#	else {
		open (VF, ">$BaseDir/$in{'topic'}.usr");
		print VF "$ENV{'REMOTE_HOST'}\n";
		close VF;
#	}

}


#############################################################
# Read in the voting file and process the votes


sub RecordVote {
	open (VF,"$BaseDir/$in{'topic'}");
	@lines = <VF>;
	chop @lines;
	close VF;
	
	$Num = @lines;
	
	# Seperate out the items
	
	$Topic = $lines[0];
	
	$Count = 1;
	
	while ($Count != $Num) {
		($Item[$Count],$VoteCount[$Count]) = split (/::/, $lines[$Count]);
		$Votes = $Votes + $VoteCount[$Count];
		$Count++;
	}
	
	$VoteCount[$in{'option'}]++;
	
	$Votes++;
	
	# Now write back the data to the file
	
	open (VF, ">$BaseDir/$in{'topic'}");
	print VF "$Topic\n";
	$Counter = 1;
	while ($Counter != $Num) {
		print VF "$Item[$Counter]::$VoteCount[$Counter]\n";
		$Counter++;
	}
	close VF;
	
	$Flag = 1;
	
	&DisplayResults;
}

sub DisplayResults {

	if ($Flag == 0) {
	
		open (VF, "$BaseDir/$in{'topic'}");
		@lines = <VF>;
		chop @lines;
		close VF;
		
		$Num = @lines;
		
		# Seperate out the items
		
		$Topic = $lines[0];
		
		$Count = 1;
		
		while ($Count != $Num) {
			($Item[$Count],$VoteCount[$Count]) = split (/::/, $lines[$Count]);
#			if ($VoteCount[$Count] != 0) {
				$Votes = $Votes + $VoteCount[$Count];
#			}
			$Count++;
		}
		
		
	}
	
	# Determine Stats
	
	$Counter = 1;
	
	if ($Votes != 0) {
		while ($Counter != $Num) {
			$Percentage[$Counter] = (int(($VoteCount[$Counter] / $Votes) * 100));
			$Counter++;
		}
	}
	
	$Counter = 1;
	$NewCount = 0;
	
	while ($Counter != $Num) {
		$NewLine[$NewCount] = "<tr bgcolor='#00CCCC'><td>$Item[$Counter]</td><td align=center>$VoteCount[$Counter]</td><td align=center>$Percentage[$Counter]\%</td></tr>\n";
		$NewCount++;
		$Counter++;
	}
		
	# Display the results
	
	$Counter = 1;

	&PrintHeader ("Results for $Topic");
	print "$Header\n";
	print "<center><h2>$Topic</h2></center>\n";
	print "<center><b>统计: $Votes</b></center><p>\n";
	print "<center><table width='200' border=0 cellspacing=1 cellpadding=5 bgcolor=#000000>\n";
	print "<tr bgcolor='#FFFFCC'><td align=center><b>选择:</b></td><td align=center><b>票数</b></td><td align=center><b>百分比:</b></td></tr>\n";
	
	foreach $Line (@NewLine) {
		print "$Line";
	}
	print "</table></center>\n";
	print "<p>\n";
	print "$Footer\n";
	print "</body>\n";
	print "</html>\n";
}
	

sub Error {
	local ($Error) = @_;
	if ($Error eq "No Duplicate Voting") {
		&PrintHeader ("No Duplicate Voting");
		print "$Header\n";
		print "<center><img src='/eek.gif'><h3><font color='red'>错误：重复投票</font></h3>\n";
		print "<center>不可重复投票！</center><p>\n";
		print "$Footer\n";
		exit;
	}
	if ($Error eq "No Option Specified") {
		&PrintHeader ("No Option Specified");
		print "$Header\n";
		print "<center><img src='/eek.gif'><h3><font color='red'>错误：没有投票</font></h3></center>\n";
		print "<center>您并没有选择票选对象。请通知管理员这个错误。</center>\n";
		print "$Footer\n";
		exit;
	}
	if ($Error eq "No Topic Specified") {
		&PrintHeader ("No Topic Specified");
		print "$Header\n";
		print "<center><img src='eek.gif'><h3><font color='red'>错误：没有主题的投票</font></h3></center>\n";
		print "<center>请告诉管理员这个错误。</center>\n";
		print "$Footer\n";
		exit;
	}
}


sub UnWeb {

   # Get the input
   # Split the name-value pairs
   @pairs = split(/&/, $ENV{'QUERY_STRING'});

   foreach $pair (@pairs) {
      ($name, $value) = split(/=/, $pair);

      # Un-Webify plus signs and %-encoding
      $value =~ tr/+/ /;
      $value =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C", hex($1))/eg;
      $value =~ s/<!--(.|\n)*-->//g;

      if ($allow_html != 1) {
         $value =~ s/<([^>]|\n)*>//g;
      }
      else {
         unless ($name eq 'body') {
	    $value =~ s/<([^>]|\n)*>//g;
         }
      }

      $in{$name} = $value;
   }

}


#######################
# Print HTML Header

sub PrintHeader {
	local ($Title) = @_;

	print "Content-type: text/html\n\n";
	print "<html>\n";
	print "	<head>\n";
	print "		<title>$Title</title>\n";
	print "	</head>\n";
}
