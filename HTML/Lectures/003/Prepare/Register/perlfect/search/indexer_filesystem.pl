
# Currently empty.
sub init_filesystem {
}

# Recursively traverse the filesystem, but ignore the files on @no_index.
sub crawl_filesystem {
  my $dir = $_[0];
  my $doc_id;
  my $file;
  
  print $dir,"\n";

  chdir $dir or (warn "Cannot chdir $dir: $!" and next);
  opendir(DIR, $dir) or (warn "Cannot open $dir: $!" and next);
  my @contents = readdir DIR;
  closedir(DIR);

  my @dirs  = grep {-d and not /^\.{1,2}$/} @contents; 
  my @files = grep {-f and /^.+\.(.+)$/ and grep {/^\Q$1\E$/} @EXT} @contents;
  
  FILE: foreach my $f (@files) {
    $file = $dir."/".$f;
    $file =~ s/\/\//\//og;

    next FILE if( to_be_ignored($file) );
    $doc_id = record_file($file);

    # loading the file:
    my $buffer = "";
    undef $/;
    open(FILE, $file) or (warn "Cannot open '$file': $!" and $DN-- and next);
    binmode(FILE);		# for reading PDF files under Windows NT
    $buffer = <FILE>;
    close(FILE); 
    $/ = "\n";  
    $buffer = parse_pdf($buffer, $file);
    if( ! $buffer ) {
      $DN--;
      next;
    }
    index_file($file, $doc_id, $buffer);
  }

  DIR: foreach my $d (@dirs) {
    $file = $dir."/".$d;
    $file =~ s/\/\//\//og;
    
    foreach my $regexp (@no_index) {
      next DIR if $file =~ /^$regexp$/;
    }
    crawl_filesystem($file);
  }
}

1;
