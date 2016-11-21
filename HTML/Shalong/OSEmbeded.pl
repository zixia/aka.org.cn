#给多人发送 AKA OS&Embeded沙龙 通知程序
use Mail::Sender;

#Email 功能初始化 及 读入通知正文
$sender = new Mail::Sender
              {smtp => '166.111.26.54', from => 'aka@aka.org.cn'};

open (TONGZHI, "tongzhi.txt") || die ("Could not open tongzhi file\n");
@tongzhi = <TONGZHI>;
$subject = shift(@tongzhi);

#发送给 沙龙所有成员
open (XCLIST, "OSEmbeded.txt") || die ("Could not open OSEmbeded.txt\n");
@xclist = <XCLIST>;

$prologue = "hi, dear friend:\n\n    这是阿卡 OSEmbeded 沙龙的通知，欢迎您能如约前来。\n    由于 egroups 的损坏，我们现在只能手工给大家发信。\n\n    如果你有沙龙技术问题和相关事务，请与 xuas\@263.net 联系\n    如你希望退出该列表，请给 freeAKA\@263.net 发信，注明<退出 OS 沙龙>，谢谢。\n    如果你有朋友希望加入该列表，也请给 freeAKA\@263.net 发信，注明<加入 OS 沙龙>，谢谢。\n\nAKA\n===============================\n";
unshift(@tongzhi, $prologue);

$signature = "\n\n====================================\n自由、协作、创造 — 为了明天\n欢迎访问：http://www.aka.org.cn  http://www.oshack.net\n“来自大雪山的大雁阿卡”\n";
push(@tongzhi, $signature);

$count = 1;
while ($count <= @xclist) {
  $email = $xclist[$count-1];
  chomp($email);
  printf (".");  if ($count % 10 == 0) { printf ("$count"); }
  $count++;

  $sender->Open({to => $email, subject => $subject});
  $sender->Send(@tongzhi);
  $sender->Close;

  sleep(2);
}
close(XCLIST);
printf ("宣传邮件地址表 Finished!\n");

#结束
close(TONGZHI);
printf ("发送通知工作结束！！！     按 Enter 键结束\n");
$var = <STDIN>;