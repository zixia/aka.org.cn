#! /usr/bin/perl -w

use CGI;

$query = new CGI;

@names = $query->param;

open( FD, ">>Student.xml" ) or die "error open xml file";
print FD "<Student>\n";
foreach $name ( @names ){
	print FD "\t<$name>" . $query->param($name) . "</$name>\n";
}
print FD "</Student>\n";


print $query->header;
print "<h1>感谢您的参与，谢谢！</h1><br><br><br>";
print "点击<a href=\"http://www.aka.org.cn\">这里</a>，返回 AKA 主页";

