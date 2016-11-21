#! /usr/bin/perl -w

use HTML::Parser ;
use CGI;

$q = new CGI;

@names = $q->param;

open( FD, ">>Student.xml" ) or die "error open xml file";
print FD "<Student>\n";
foreach $name ( @names ){
	print FD "\t<$name>" . $q->param($name) . "</$name>\n";
}
print FD "</Student>\n";


print $q->header;
print "<h1>感谢您的参与，谢谢！</h1><br><br><br>";
print "点击<a href=\"http://www.aka.org.cn\">这里</a>，返回 AKA 主页";


$p = HTML::Parser->new ;

@Students = ();
%Vote = ();

$p->parse_file ("/home/akaWWW/cgi-bin/Student.xml") ;


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
