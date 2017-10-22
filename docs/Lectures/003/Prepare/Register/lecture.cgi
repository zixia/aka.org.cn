#! /usr/bin/perl -w

use HTML::Parser ;
use CGI;
use Fcntl ':flock';

$q = new CGI;
print $q->header;

@names = $q->param;

if( $q->param('select') eq '提交' ){
	open( FD, ">>Student.xml" ) or die "error open xml file";
	flock( FD, LOCK_EX );
	print FD "<Student>\n";
	foreach $name ( @names ){
		next if ( $name eq 'select' );
		print FD "\t<$name>" . $q->param($name) . "</$name>\n";
	}
	print FD "</Student>\n";
	flock( FD, LOCK_UN );
	close( FD );
}

print "<h1>感谢您的参与，谢谢！</h1><br>";
print "点击<a href=\"http://www.aka.org.cn\">这里</a>，返回 AKA 主页";


$p = HTML::Parser->new ;

@Students = ();
%Vote = ();

$p->parse_file ("/home/akaWWW/cgi-bin/Student.xml") ;


print "<hr><br><center>最新调查结果<center><br>";

print "<table width=750 border=0><tr bgcolor='999999'><td width=150>对以前的讲座是否满意</td><td width=150>本期最感兴趣的讲座</td><td width=150>来自大学或公司</td><td width=150>年龄分布（以30岁为界）</td><td width=150>巾帼不让须眉</td></tr></table>";
print "<table border=0><tr>";
foreach $item ( keys %Vote ){
	print "<td><table width=150 border=0>";
	foreach ( keys %{$Vote{$item}} ){
		print "<tr><td>$_</td><td>&nbsp&nbsp" . $Vote{$item}{$_} . "&nbsp&nbsp</td></tr>";
	}
	print "</table></td>";
}
print "</tr></table>\n";
		
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
	if( $text eq "提交" || $text eq '看结果' ){
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
