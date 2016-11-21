#!/usr/bin/perl

$pptNamePrefix = "»ÃµÆÆ¬";
$newNamePrefix = "AFS";
$pages = 37;
$TEMPLATE = "temp.html";

for ($cnt=1; $cnt<=$pages; $cnt++) {
  $currStr = &get0Str($cnt);
  `mv $pptNamePrefix$cnt".GIF" $currStr".gif"`;

  $prevStr = &get0Str($cnt-1);
  $nextStr = &get0Str(($cnt % $pages) + 1);

  &createfile($currStr, $prevStr, $nextStr);
}

sub get0Str {
  my($num) = @_;
  $str = $newNamePrefix;
  if ($num < 100) {
    $str = $str."0";
  }
  if ($num < 10) {
    $str = $str."0";
  }
  $str = $str.$num;
}

sub createfile {
  my($curr, $prev, $next) = @_;

  open(TEMPFILE, $TEMPLATE);
  open(NEWFILE, ">$curr.html");
  while ($line=<TEMPFILE>) {
    $line =~ s/previous.html/$prev.html/g;
    $line =~ s/next.html/$next.html/g;
    $line =~ s/current.gif/$curr.gif/g;
    print NEWFILE ($line);
  }
  close(NEWFILE);
  close(TEMPFILE);
}

