<?
	include("key.php");
	$id = GetID();
	if(!$id)exit("请先登录");

?>
您好，您的账号是:<?= $id?>