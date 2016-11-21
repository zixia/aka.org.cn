<?

 $ZKeyPKey="THIS_IS_A_RANDOM_STRING_qeu82kmssd";

 function GetID()
 {
        global $ZKEY,$ZKeyPKey;

        if(strlen($ZKEY)<40)return "";

        $zks = explode('-',$ZKEY);

        $ID = $zks[0];
        $TM = substr($zks[1],0,strlen($zks[1])-3)+0;
        $MDK = $zks[2];

        if(($tm = time())-$TM>3600)return "";

        if(md5($ID."-".$zks[1]."-".$ZKeyPKey)!=$MDK)return "";

        $newkey = $ID."-".$tm."000-".md5($ID."-".$tm."000-".$ZKeyPKey);

        setcookie("ZKEY",$newkey,$tm+36000,"",".mydomain.com");

        return $ID;
 }

 function NewID($ID)
 {
        global $ZKEY,$ZKeyPKey;

        $tm = time();

        $newkey = $ID."-".$tm."000-".md5($ID."-".$tm."000-".$ZKeyPKey);

        setcookie("ZKEY",$newkey,$tm+36000,"",".mydomain.com");

        return $newkey;
 }

?>
------=_NextPart_000_000C_01C03178.27F702F0
Content-Type: application/octet-stream;
	name="login.php"
Content-Transfer-Encoding: quoted-printable
Content-Disposition: attachment;
	filename="login.php"

<?
	include("sql.php");
	include("key.php");
	$err =3D "";
	if($add)
	{
		$mysql =3D mysql_connect($SQLHOST,$SQLUSER,$SQLPASS);
		if(!$mysql)
		{
			exit("=CA=FD=BE=DD=BF=E2=B7=B1=C3=A6=A3=AC=C7=EB=C9=D4=BA=F3");
		}
		else
		{
			$uid =3D strtolower($uid);
			$userPasswd =3D mysql_fetch_row(mysql_db_query('users',"SELECT =
MD5PASS FROM users WHERE USERID=3D'$uid'",$mysql));
			if(!$userPasswd || strlen($userPasswd[0])!=3D32)
				$err .=3D "=D3=C3=BB=A7=B2=BB=B4=E6=D4=DA<BR>";
			else
			{
				if(md5($pass)!=3D $userPasswd[0])
					$err .=3D "=C3=DC=C2=EB=B2=BB=D5=FD=C8=B7<br>";
				else
				{
					NewID($uid);
					mysql_db_query('users',"UPDATE users SET LASTIP=3D'$REMOTE_ADDR' =
WHERE USERID=3D'$uid'",$mysql);
					echo "=B5=C7=C2=BC=B3=C9=B9=A6=A3=A1";
				}
			}
			mysql_close($mysql);
		}
	}

?>
<?if($err)echo $err?>;
<FORM METHOD=3DPOST>
<INPUT TYPE=3DHIDDEN NAME=3Dadd VALUE=3D1>
=D5=CB=BA=C5 <INPUT NAME=3Duid><BR>
=C3=DC=C2=EB <INPUT NAME=3Dpass TYPE=3Dpassword><BR>
<INPUT TYPE=3DSUBMIT VALUE=3D'=B5=C7=C2=BC'>
</FORM>
------=_NextPart_000_000C_01C03178.27F702F0
Content-Type: application/octet-stream;
	name="passwd.php"
Content-Transfer-Encoding: quoted-printable
Content-Disposition: attachment;
	filename="passwd.php"

<?
	include("sql.php");
	include("key.php");
	$id =3D GetID();
	if(!$id)exit("=C7=EB=CF=C8=B5=C7=C2=BC");
	if($add)
	{
		if(strlen($pass1)<4)
		{
			$err .=3D =
"=C3=DC=C2=EB=B3=A4=B6=C8=B1=D8=D0=EB=B4=F3=D3=DA4=B8=F6=D7=D6";
			$pass1 =3D "";
		}
		if($pass1 !=3D $pass2)
		{
			$err .=3D =
"=CA=E4=C8=EB=B5=C4=C1=BD=B4=CE=C3=DC=C2=F0=B2=BB=C6=A5=C5=E4";
			$pass1 =3D "";
		}
		if(!$err)
		{
			$mysql =3D mysql_connect($SQLHOST,$SQLUSER,$SQLPASS);
			if(!$mysql)
			{
				$err =3D "=CA=FD=BE=DD=BF=E2=B7=B1=C3=A6=A3=AC=C7=EB=C9=D4=BA=F3";
			}
			else
			{
				$userPasswd =3D mysql_fetch_row(mysql_db_query('users',"SELECT =
MD5PASS FROM users WHERE USERID=3D'$id'",$mysql));
				if(md5($passs)!=3D $userPasswd[0])
					$err .=3D "=C3=DC=C2=EB=B2=BB=D5=FD=C8=B7<br>";
				else
				{
					$md5pass =3D md5($pass1);
					mysql_db_query('users',"UPDATE users SET MD5PASS=3D'$md5pass' WHERE =
USERID=3D'$id'",$mysql);
				}
				mysql_close($mysql);
			}
		}
		if(!$err)
		{
			echo "=D0=DE=B8=C4=B3=C9=B9=A6";
			die();
		}
	}
?>
<FORM METHOD=3DPOST>
<?if($err)echo $err?>;
<INPUT TYPE=3DHIDDEN NAME=3Dadd VALUE=3D1>
=D4=AD=C3=DC=C2=EB<INPUT NAME=3Dpasss type=3Dpassword>
=D0=C2=C3=DC=C2=EB<INPUT NAME=3Dpass1 type=3Dpassword>
=D6=D8=B8=B4=CA=E4=C8=EB<INPUT NAME=3Dpass2 type=3Dpassword>
<INPUT TYPE=3DSUMBIT VALUE=3D'=C9=EA=C7=EB'>
</FORM>
------=_NextPart_000_000C_01C03178.27F702F0
Content-Type: application/octet-stream;
	name="register.php"
Content-Transfer-Encoding: quoted-printable
Content-Disposition: attachment;
	filename="register.php"

<?
if($add)
{
	include("sql.php");
	$uid =3D strtolower($uid);
	$err =3D "";
	if(!ereg("^[a-z]{3,24}$",$uid))$err .=3D =
"=D3=C3=BB=A7=D5=CB=BA=C5=B1=D8=D0=EB=CA=C7a-z=B5=C4=D7=D6=B7=FB=A3=AC=B2=
=BB=C4=DC=B3=AC=B9=FD24=D7=D6=B7=FB=C7=D2=B2=BB=C4=DC=C9=D9=D3=DA3=B8=F6=D7=
=D6=B7=FB<br>";
	if(!ereg("^[0-9]{15,18}$",$idnum))$err .=3D =
"=D3=C3=BB=A7=D5=CB=BA=C5=B1=D8=D0=EB=CA=C7a-z=B5=C4=D7=D6=B7=FB=A3=AC=B2=
=BB=C4=DC=B3=AC=B9=FD24=D7=D6=B7=FB=C7=D2=B2=BB=C4=DC=C9=D9=D3=DA3=B8=F6=D7=
=D6=B7=FB<br>";
	if(strlen($pass1)<4)
	{
		$err .=3D =
"=C3=DC=C2=EB=B3=A4=B6=C8=B1=D8=D0=EB=B4=F3=D3=DA4=B8=F6=D7=D6";
		$pass1 =3D "";
	}
	if($pass1 !=3D $pass2)
	{
		$err .=3D =
"=CA=E4=C8=EB=B5=C4=C1=BD=B4=CE=C3=DC=C2=F0=B2=BB=C6=A5=C5=E4";
		$pass1 =3D "";
	}
	if(!$err)
	{
		$mysql =3D mysql_connect($SQLHOST,$SQLUSER,$SQLPASS);
		if(!$mysql)
		{
			$err =3D "=CA=FD=BE=DD=BF=E2=B7=B1=C3=A6=A3=AC=C7=EB=C9=D4=BA=F3";
		}
		else
		{
			$TestUser =3D mysql_fetch_row(mysql_db_query('users',"SELECT COUNT(*) =
FROM users WHERE USERID=3D'$uid'",$mysql));
			if($TestUser[0]!=3D0)$err .=3D "=D5=CB=BA=C5 $uid =
=D2=D1=BE=AD=B4=E6=D4=DA=A3=AC=C7=EB=BB=BB=D2=BB=B8=F6=D5=CB=BA=C5=C3=FB=B3=
=C6=D4=D9=CA=D4<br>";
			else
			{
				$md5pass =3D md5($pass1);
				mysql_db_query('users',"INSERT INTO users SET =
USERID=3D'$uid',MD5PASS=3D'$md5pass',IDNUMBER=3D'$idnum',MAIL=3D'$email',=
LASTIP=3D'$REMOTE_ADDR'",$mysql);
			}
			mysql_close($mysql);
		}
	}
	if(!$err)
	{
		echo "=D7=A2=B2=E1=B3=C9=B9=A6";
		die();
	}
}
?>
<?if($err)echo $err?>
=C9=EA=C7=EB=D0=C2=D5=CB=BA=C5
<FORM method=3Dpost>
<INPUT TYPE=3DHIDDEN NAME=3Dadd VALUE=3D1>
=D5=CB=BA=C5=C3=FB=B3=C6 <INPUT NAME=3Duid VALUE=3D'<?=3D $uid?>'><BR>
=D3=C3=BB=A7=C3=DC=C2=EB <INPUT NAME=3Dpass1 type=3Dpassword =
VALUE=3D'<?=3D $pass1?>'><BR>
=D6=D8=B8=B4=C3=DC=C2=EB <INPUT NAME=3Dpass2 type=3Dpassword =
VALUE=3D'<?=3D $pass1?>'><BR>
=C9=ED=B7=DD=D6=A4=BA=C5 <INPUT NAME=3Didnum  VALUE=3D'<?=3D =
$idnum?>'><BR>
=B5=E7=D7=D3=D3=CA=BC=FE <INPUT NAME=3Demail  VALUE=3D'<?=3D =
$email?>'><BR>
<INPUT TYPE=3DSUMBIT VALUE=3D'=C9=EA=C7=EB'>
</FORM>
------=_NextPart_000_000C_01C03178.27F702F0
Content-Type: application/octet-stream;
	name="sql.php"
Content-Transfer-Encoding: 7bit
Content-Disposition: attachment;
	filename="sql.php"

<?
SQLHOST='localhost';
SQLUSER='webuser';
SQLPASS='myweb';
?>
------=_NextPart_000_000C_01C03178.27F702F0
Content-Type: application/octet-stream;
	name="test.php"
Content-Transfer-Encoding: base64
Content-Disposition: attachment;
	filename="test.php"

PD8NCglpbmNsdWRlKCJrZXkucGhwIik7DQoJJGlkID0gR2V0SUQoKTsNCglpZighJGlkKWV4aXQo
Isfrz8i1x8K8Iik7DQoNCj8+DQrE+rrDo6zE+rXE1cu6xcrHOjw/PSAkaWQ/Pg==

------=_NextPart_000_000C_01C03178.27F702F0--

