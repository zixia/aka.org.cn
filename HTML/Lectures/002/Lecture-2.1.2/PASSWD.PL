#!/usr/bin/perl

$password="aka"; #密码为aka

printf "请问您的姓名?  ";
$name=<STDIN>;
chop($name);

if ($name eq "aka"){
  printf "欢迎 AKAer 进入本系统!\n请输入密码：";
  $guess=<STDIN>;
  chop($guess);
 
  while ($guess ne $password){
    printf "密码错误，请重新输入：";
    $guess=<STDIN>;
    chop($guess);
  }
}
