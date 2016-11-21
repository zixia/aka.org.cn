#!/bin/usr/perl

print "Content-type: text/html\n\n"; 
print "<TITLE>WWWQuiz</TITLE>";
print "<h1>在线测验</h1>";


&parse; 

%ANSWER=("1","A","2","C","3","C","4","C","5","B");
$right=0;
$wrong=0; 

foreach $Num (sort keys %FORM)
{
  print "第 $Num 题你所答的答案是：$FORM{$Num}　";
  print "标准答案：$ANSWER{$Num}<br>"; 

  if ($FORM{$Num} eq $ANSWER{$Num})
  {
    $right++;
  }
  else
  {
    $wrong++;
  }
}

print "<p>您总共答对：$right 题；答错：$wrong 题<p>";

sub parse{  

  read(STDIN, $buffer, $ENV{'CONTENT_LENGTH'}); 

  @pairs = split(/&/, $buffer); 

  foreach $pair (@pairs){
    ($name, $value) = split(/=/, $pair);
    $value =~ tr/+/ /;
    $value =~ s/%([a-fA-F0-9][a-fA-F0-9])/pack("C", hex($1))/eg;
    $FORM{$name} = $value;
  }
}
