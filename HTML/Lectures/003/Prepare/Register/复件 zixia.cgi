#!/usr/bin/perl

use HTML::Parser ;
use CGI;

$q = new CGI;
print $q->header;

$p = HTML::Parser->new ;

@Students = ();
%Vote = ();

$p->parse_file ("/home/akaWWW/cgi-bin/Student.xml") ;



print "<html><head><title>AKA注册统计</title></head><body>";
print "<table border=1>";
print "<tr bgcolor='888888'><td>姓名</td><td>email</td><td>联系方法</td></tr>";
foreach $Student ( @Students ){
	print "<tr><td>${$Student}{username}</td><td>${$Student}{email}</td><td>${$Student}{address}</td></tr>";
#print ${$Student}{username}, "\n";
}
print "</table>";
print "<hr>";

print "<table border=1>";
foreach $item ( keys %Vote ){
	print "<tr bgcolor='999999'><td colspan=4>$item</td></tr>";
	foreach ( keys %{$Vote{$item}} ){
		print "<tr><td>$_</td><td>" . $Vote{$item}{$_} . "</td></tr>";
	}
}
print "</table>\n";
		
print "</body></html>";
exit;


sub HTML::Parser::start{
	local ($self,$tag, $attr, $attrseq, $origtext) = @_ ;
	$self->{tag} = $tag;
}

sub HTML::Parser::text{
	local ($self,$text) = @_ ;
	if( ! $self->{tag} ){
		return;
	}
	if( ! $text ){
		return;
	}
	if ($self->{tag} eq "username"){
		$Student{$self->{tag}} = $text;
	}elsif ($self->{tag} eq "email"){
		$Student{$self->{tag}} = $text;
	}elsif ($self->{tag} eq "address"){
		$Student{$self->{tag}} = $text;
	}elsif( $self->{tag} ne "student" ){
		$Vote{$self->{tag}}{$text}++;
	}
}

sub HTML::Parser::end{
	local ($self,$tag, $origtext) = @_ ;
	$self->{tag} = 0;
	#print "tag: $tag end\n" ;
	if( $tag eq "student" ){
		return unless $Student{username};
		local %tmpStudent;
		%tmpStudent = %Student;
		%Student = ();
		push( @Students, \%tmpStudent );
	}
}

