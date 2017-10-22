#!/usr/bin/perl 

# 输出 HTML 说明信息，让浏览器知道这是一个 HTML 格式的文档。
print "Content-type: text/html\n\n"; 
# 打开文件 count.txt 供读取，count.txt 文件中存储了一个整数。
open(CNT,"count.txt"); 

# 读取 count.txt 文件中的整数，存放到变量 $count 中。CNT 相当于 C 语言中的文件句柄。
$count=<CNT>; 
# 如果该程序被执行，说明调用该程序的页面又被访问了一次，变量 $count 加 1。
$count ++; 

# 把加 1 后的结果，和访问者的 IP 地址输出给浏览器。
print "你是第 $count 位访问者，来自 $ENV{'REMOTE_ADDR'} <br>"; 

# 关闭 count.txt 文件
close(CNT); 

# 再打开另一个文件 count.old 用于永久保存加 1 后的计数值。（因为 $count 在程序执行结束后就没有了）
open(OLD ,">count.old"); 
# 将变量 $count，即加 1 后的结果写回 count.old 文件中。
print OLD $count; 
# 关闭 count.old 文件
close(OLD); 

# 执行 Shell 命令，将 count.old 文件复制成 count.txt 文件，此时这两个文件中都包含了最新的计数值。
`cp count.old count.txt`;

# 退出该程序
exit;
