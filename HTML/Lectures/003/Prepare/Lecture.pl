#给多人发送 AKA 通知程序
use Mail::Sender;

#Email 功能初始化 及 读入通知正文
$sender = new Mail::Sender
              {smtp => '166.111.26.54', from => 'aka@aka.org.cn'};

open (TONGZHI, "tongzhi.txt") || die ("Could not open tongzhi file\n");
@tongzhi = <TONGZHI>;
$subject = shift(@tongzhi);

$prologue = "\n    阿卡活动的通知，欢迎您能如约前来。\n    如你希望退出该列表，请给 freeAKA\@263.net 发信，注明<退出 AKA 活动通知>，谢谢。\n    如果你有朋友希望加入该列表，也请给 freeAKA\@263.net 发信，注明<加入 AKA 活动通知>，谢谢。\n\nAKA\n\n===============================\n";
unshift(@tongzhi, $prologue);
$signature = "\n\n====================================\n自由、协作、创造 — 为了明天\n欢迎访问：http://www.aka.org.cn \n“来自大雪山的大雁阿卡”\n";
push(@tongzhi, $signature);

#发送给报名邮件列表
open (BMLIST, "报名的邮件列表.txt") || die ("Could not open 报名的邮件列表\n");
@bmlist = <BMLIST>;

$count = 1;
while ($count <= @bmlist) {
  $email = $bmlist[$count-1];
  chomp($email);
  printf (".");  if ($count % 10 == 0) { printf ("$count"); }
  $count++;

  my @temp = @tongzhi;
  my $to = "To: ".$email."\n\n";
  unshift(@temp, $to);
  $sender->Open({to => $email, subject => $subject});
  $sender->Send(@temp);
  $sender->Close;

  sleep(2);
}
close(BMLIST);
printf("报名的邮件列表 Finished!\n\n");

#发送给 宣传地址列表
open (XCLIST, "宣传邮件地址表.txt") || die ("Could not open 宣传邮件地址表\n");
@xclist = <XCLIST>;

$prologue = "hi, dear friend:\n\n    这是阿卡活动通知，如你感兴趣，欢迎您来参加。\n    如有可能，请帮忙把该信息转贴到网络上的其他相关讨论组，谢谢。\n\n";
unshift(@tongzhi, $prologue);

$count = 1;
while ($count <= @xclist) {
  $email = $xclist[$count-1];
  chomp($email);
  printf (".");  if ($count % 10 == 0) { printf ("$count"); }
  $count++;

  my @temp = @tongzhi;
  my $to = "To: ".$email."\n\n";
  unshift(@temp, $to);
  $sender->Open({to => $email, subject => $subject});
  $sender->Send(@temp);
  $sender->Close;

  sleep(2);
}
close(XCLIST);
printf ("宣传邮件地址表 Finished!\n\n");

#结束
close(TONGZHI);
printf ("发送通知工作结束！！！     按 Enter 键结束\n");
$var = <STDIN>;