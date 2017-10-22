#!/usr/bin/perl
#$rcs = ' $Id: search.pl,v 1.22 2000/11/07 00:09:53 daniel Exp $ ' ;

# Perlfect Search
#
# Copyright (C) 1999-2000 Giorgos Zervas <giorgos@perlfect.com>
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
# USA

# Use the next two lines to log how long searches take:
#use Time::HiRes qw ();
#my $start_time = [Time::HiRes::gettimeofday];

use CGI;
use CGI::Carp qw(fatalsToBrowser);
use URI::Escape;
use Fcntl;

# added program path to @INC because it fails to find ./conf.pl if started from
# other directory
{ 
  # block is for $1 not mantaining its value
  $0 =~ /(.*)(\\|\/)/;
  push @INC, $1 if $1;
}
require Perlfect::Template;

my $db_package = "";
# To use tainting, comment in the next 2 lines and comment out the next 8 lines.
# Note that you also have to add "./" to the filenames in the require commands.
#use DB_File;
#$db_package = 'DB_File';
package AnyDBM_File;
@ISA = qw(DB_File GDBM_File SDBM_File ODBM_File NDBM_File) unless @ISA;
foreach my $isa (@ISA) {
  if( eval("require $isa") ) {
    $db_package = $isa;
    last;
  }
}

package main;
require 'conf.pl';
require 'SGMLEntities.pl';

my %inv_index_db;
my %docs_db;
my %sizes_db;
my %desc_db;
my %titles_db;
my %terms_db;

tie %inv_index_db, $db_package, $INV_INDEX_DB_FILE, O_RDONLY, 0755 or die "Cannot open $INV_INDEX_DB_FILE: $!";   
tie %docs_db,      $db_package, $DOCS_DB_FILE, O_RDONLY, 0755      or die "Cannot open $DOCS_DB_FILE: $!";   
tie %sizes_db,     $db_package, $SIZES_DB_FILE, O_RDONLY, 0755     or die "Cannot open $SIZES_DB_FILE: $!";   
tie %desc_db,      $db_package, $DESC_DB_FILE, O_RDONLY, 0755      or die "Cannot open $DESC_DB_FILE: $!";   
tie %content_db,   $db_package, $CONTENT_DB_FILE, O_RDONLY, 0755   or die "Cannot open $CONTENT_DB_FILE: $!"; 
tie %titles_db,    $db_package, $TITLES_DB_FILE, O_RDONLY, 0755    or die "Cannot open $TITLES_DB_FILE: $!";   
tie %terms_db,     $db_package, $TERMS_DB_FILE, O_RDONLY, 0755     or die "Cannot open $TERMS_DB_FILE: $!";   

my (@force, @not, @other);
my (@docs, @valid_docs);
my %answer;

build_char_string();
main();
exit;

sub main {
  # initialize everything with empty values (because we might run under mod_perl)
  @force = ();
  @not = ();
  @other = ();
  @docs = ();
  @valid_docs = ();
  %answer = ();
  my $query = new CGI;
  if (create_query($query)) { #if some valid documents exist
    apply_booleans();
    answer_query();
  }  
  my $html = cast_template($query);
  print $query->header;
  print $html;
  log_query($query);
}

sub answer_query {
  my @term_ids = (@force, @other);

  my %valid_docs = ();
  map { $valid_docs{$_} = 1 } @valid_docs;

  foreach my $term_id (@term_ids) {
    my %v = unpack('S*', $inv_index_db{$term_id});
    foreach my $doc_id (keys %v) {
      if( exists $valid_docs{$doc_id} ) {
        my $boost = $answer{$doc_id};
        $answer{$doc_id} += $v{$doc_id};
        $answer{$doc_id} *= $MULTIPLE_MATCH_BOOST if( $MULTIPLE_MATCH_BOOST && $boost );
      }
    }
  }
}

sub apply_booleans {
  #locate the valid documents by applying the booleans
  my ($term_id, $doc_id, $first_doc_id);
  my %v = ();
  my @ary = ();
  my @not_docs = ();

  my %not_docs = ();
  map { $not_docs{$_} = 1} @not_docs;

  foreach $term_id (@not) {
    %v = unpack("S*", $inv_index_db{$term_id});
    foreach $doc_id (keys %v) {
      push @not_docs, $doc_id unless exists $not_docs{$doc_id};
    }
  }
  
  if (@force) {
    $first_doc_id = pop @force;
    %v  = unpack("S*", $inv_index_db{$first_doc_id});
    @valid_docs = keys %v; 
    foreach $term_id (@force) {
      %v = unpack("S*", $inv_index_db{$term_id});
      @ary = keys %v;
      @valid_docs = intersection(\@valid_docs, \@ary);
    }
    push @force, $first_doc_id;
  } else {
    @valid_docs = keys %docs_db;
  }

  @valid_docs = minus(\@valid_docs, \@not_docs);
}

sub create_query {
  my $query = shift;
  my $query_str = cleanup($query->param('q'));
  my $mode = cleanup($query->param('mode'));
  my @terms = split " ", $query_str;
  my $buffer;
  my ($sterm, $nterm);
  
  foreach my $term (@terms) {
    $buffer = normalize($term);
    foreach my $nterm (split " ",$buffer) {
      $sterm = stem($nterm);
      # For "Match all words" just add a "+" to every term that has no operator:
      if ( $mode eq 'all' && $term !~ m/^(\+|\-)/ ) {
        $term = '+'.$term;
      }
      if ($term =~ /^\+/) {
	if ($terms_db{$sterm}) {
	  push @force, $terms_db{$sterm};
	} else {
	  return 0;	# this term was not found, we can stop already
	}
      } elsif ($term =~ /^\-/) {
	push @not, $terms_db{$sterm} if $terms_db{$sterm};
      } else {
	push @other, $terms_db{$sterm} if $terms_db{$sterm};
      }
    }
  }

  return 1;
}

# Populate the template with search results. All external data has to be
# access via cleanup(), to avoid cross site scripting attacks.
sub cast_template {
  my $query = shift;
  my %h = ();
  my $rank = 0;
  my $lang = cleanup($query->param('lang'));
  my $mode = cleanup($query->param('mode'));
  my $q = cleanup($query->param('q'));
  my $p = cleanup($query->param('p'));
  $lang = $DEFAULT_LANG if( ! ($lang && $SEARCH_TEMPLATE{$lang} && $NO_MATCH_TEMPLATE{$lang}) );
  my $file;
  if( keys(%answer) == 0 ) {
    $file = $NO_MATCH_TEMPLATE{$lang};
  } else {
    $file = $SEARCH_TEMPLATE{$lang};
  }
  my $template = new Perlfect::Template($file);

  # %h carries values that will show up in the result page at <!--cgi: key-->:
  $h{script_name} = "Perlfect Search $VERSION";
  # normalize() should also clean HTML characters, but let's be safe and use cleanup():
  $h{query_str}   = normalize(cleanup($q));
  $h{results_num} = keys %answer;
  $h{lang} = $lang;
  if( $mode eq 'all' ) {
    $h{match_all} = " selected";
    $h{match_any} = "";
  } else {
    $h{match_all} = "";
    $h{match_any} = " selected";
  }
  
  my $current_page = $p;
  $current_page ||= 1;

  my $first = ($current_page - 1) * $RESULTS_PER_PAGE; 
  my $last  = $first + $RESULTS_PER_PAGE - 1;
  
  my $real_last = keys %answer;
  if ($last >= $real_last) {
    $last = $real_last - 1;
  }
  
  my $percent_factor;
  if( $PERCENTAGE_RANKING ) {
    my $max_score = 0;
    foreach my $doc_ranking (values %answer) {
      $max_score = $doc_ranking if( $doc_ranking > $max_score );
    }
    $percent_factor = 100/$max_score if( $max_score );
  }
  foreach ((sort {$answer{$b} <=> $answer{$a}} keys %answer)[$first..$last]) {
    my $score = $answer{$_};
    if( $PERCENTAGE_RANKING ) {
      $score = sprintf("%.f\%", $score*$percent_factor);
    } else {
      $score = sprintf("%.2f", $score/100);
    }
    my $desc = get_summary($_, $q);
    $template->cast_loop ("results", [{rank => $first+(++$rank), 
				       url => $BASE_URL.uri_escape($docs_db{$_}, '^A-Za-z0-9\-_.~\/'), 
				       visibleurl => $BASE_URL.$docs_db{$_}, 
				       title => $titles_db{$_}, 
				       score => $score,
				       description => $desc,
				       size => sprintf("%.0f", $sizes_db{$_}/1000) || 1,
				      }]);
  }
  $template->finalize("results");
  
  my $last_page = ceil($real_last, $RESULTS_PER_PAGE);
  $last_page ||= 1;
  my $query_str = CGI::escape($q);
  $lang = CGI::escape($lang);
  # keep order of arguments as in the search page to get correct visited link recognition:
  my $queries = "lang=$lang&amp;mode=$mode&amp;q=$query_str";	# using "&amp;" is correct, "&" isn't
  if ($current_page == 1) {
    $h{previous} = "";
    if ($last_page > $current_page) {
      $h{next} = "<A href=\"$SEARCH_URL?p=2&amp;$queries\">$NEXT_PAGE{$lang}</A>";  
    } else {
      $h{next} = "";
    }
  } elsif ($current_page == $last_page) {
    $h{previous} = "<A href=\"$SEARCH_URL?p=".($last_page-1)."&amp;$queries\">$PREV_PAGE{$lang}</A>";  
    $h{next} = "";
  } else {
    $h{previous} = "<A href=\"$SEARCH_URL?p=".($current_page-1)."&amp;$queries\">$PREV_PAGE{$lang}</A>";  
    $h{next} = "<A href=\"$SEARCH_URL?p=".($current_page+1)."&amp;$queries\">$NEXT_PAGE{$lang}</A>";  
  }
  
  for (1..$last_page) {
    if ($_ != $current_page) {
      $h{navbar} .= "<A href=\"$SEARCH_URL?p=$_&amp;$queries\">$_</A> ";
    } else {
      $h{navbar} .= "<STRONG>$_</STRONG> ";
    }
  }

  $h{current_page} = $current_page;
  $h{total_pages}  = $last_page;
  $h{search_url}   = $SEARCH_URL;

  $template->cast(\%h);
  return $template->html;
}

sub normalize {
  my $buffer = $_[0];

  $buffer =~ s/-(\s*\n\s*)?//g; # join parts of hyphenated words

  if( $SPECIAL_CHARACTERS ) {
    # We don't have special characters in our index, so don't try to search for them:
    $buffer =~ s/[∆Ê]/ae/gs;
    $buffer =~ s/[ﬁ˛]/th/igs;
    $buffer =~ s/ﬂ/ss/gs;
    $buffer =~ tr/ƒ≈∆«»“…‹” Ê›‘ÀÁﬁ’Ã˙ÒËﬂ÷Õ˚ÚÈ‡Œ¸ÛÍ·ÿœ˝ÙÎ‚Ÿ–˛ıÏ„⁄—ˇˆÌ‰€¿ÓÂ¡¯Ô¬˘√/AAACEOEUOEaYOEecTOIunesOIuoeaIuoeaOIyoeaUEtoiaUNyoiaUAiaAoiAuA/;
  }

  if ($INDEX_NUMBERS) {
    $buffer =~ s/(<[^>]*>)/ /gs;
  } else {
    $buffer =~ s/(\b\d+\b)|(<[^>]*>)/ /gs;
  }

  $buffer =~ tr/a-zA-Z0-9_/ /cs;
  return lc $buffer;
}

# Returns the content of the META description tag or the content of the match,
# if $CONTEXT_SIZE is enabled:
sub get_summary {
  my $id = $_[0];
  my @terms = split(" ", normalize_special_chars($_[1]));
  # +/- operators aren't interesting here:
  foreach my $term (@terms) {
    $term =~ s/^(\+|\-)//;
  }
  my $desc;
  if( $CONTEXT_SIZE ) {
    $desc = get_context($content_db{$id}, @terms);
  }    
  if( ! $desc ) {
    $desc = $desc_db{$id};
    foreach my $term (@terms) {
      $desc = term_emphasize($desc, $term);
    }
  }
  return $desc;
}

# Get context for all the queried terms. Return "" if no context is found.
# TODO:
# -Search for "Hauser" finds "H‰user" but doesn't show context (and emphasize it)
# -Context may be repeated in case of two terms near each other
sub get_context {
  my $buf = shift;
  my @terms = @_;
  my @contexts;
  foreach my $term (@terms) {
    push(@contexts, get_context_for_term($buf, $term));
  }
  my $context = "";
  my $ct = 0;
  foreach my $result (@contexts) {
    $context .= "...".$result."...";
    $context .= "<br>" if( $ct < scalar(@contexts)-1 );
    $context .= "\n";
    $ct++;
  }
  return $context;
}

# Get up to $CONTEXT_EXAMPLES strings for a term.
sub get_context_for_term {
  my $desc = shift;
  my $term = shift;
  my @contexts = ();
  my $ct = 1;
  # find occurences of a single term:
  while( $desc =~ m/\b$term\b/igs ) {
    my $pos = pos($desc);
    last if( $ct > $CONTEXT_EXAMPLES );
    $ct++;

    # get text before that match:
    my (@desc_ary, $first, $last);
    @desc_ary = split(" ", substr($desc, 0, $pos));
    $first = scalar(@desc_ary) - int(($CONTEXT_DESC_WORDS/2));
    if( $first <= 0 ) {
      $first = 0;
    }
    $last = scalar(@desc_ary) - 1;
    my $part_one = join(" ", @desc_ary[$first..$last]);

    # get text after that match:
    @desc_ary = split(" ", substr($desc, $pos, length($desc)));
    $last = $CONTEXT_DESC_WORDS/2;
    if( $last > scalar(@desc_ary) ) {
      $last = scalar(@desc_ary)-1;
    }
    my $part_two = join(" ", @desc_ary[0..$last]);

    # join both parts and emphasize term:
    my $tmp_context = $part_one.($part_two =~ m/^\b/ ? " " : "").$part_two;
    $tmp_context = term_emphasize($tmp_context, $term);
    push(@contexts, $tmp_context);
  }
  return @contexts;
}

# Remove some HTML special characters from a string. This is necessary
# to avoid cross site scripting attacks. 
# See http://www.cert.org/advisories/CA-2000-02.html
sub cleanup {
  my $str = $_[0];
  $str =~ s/[<>"&]//igs;
  return $str;
}

sub term_emphasize {
  my $str = $_[0];
  my $term = $_[1];
  my $term_no_accents = remove_accents($term);
  $str =~ s/\b($term|$term_no_accents)\b/<strong>$1<\/strong>/igs;
  return $str;
}

sub stem {
  my $str = $_[0];
  $str = substr $str, 0, $STEMCHARS if $STEMCHARS;
  return $str;
}

sub ceil {
  my $x = $_[0];
  my $y = $_[1];

  if ($x % $y == 0) {
    return $x / $y;
  } else {
    return int($x / $y + 1);
  }
}

# Returns an array with elements that are in both @{$ra} and @{$rb}.
sub intersection {
  my ($ra, $rb) = @_;
  my @i;
  # use a hash (instead of grep) for much better speed:
  my %check = ();
  foreach my $element (@{$rb}) {
    $check{$element} = 1;
  }
  foreach my $doc_id (@{$ra}) {
    push @i, $doc_id if( $check{$doc_id} );
  }
  return @i;
}

# Returns an array with the elements of @{$ra} minus those of @{$rb}.
sub minus {
  my ($ra, $rb) = @_;
  my @i;
  # use a hash (instead of grep) for much better speed:
  my %check = ();
  foreach my $element (@{$rb}) {
    $check{$element} = 1;
  }
  foreach my $doc_id (@{$ra}) {
    push @i, $doc_id if( ! $check{$doc_id} );
  }
  return @i;
}

# Log the query in a file, using this format:
# REMOTE_HOST;date;terms;matches;current page;(time to search in seconds);
# For the last value you need to use Time::HiRes (see top of the script)
sub log_query {
  my $query = $_[0];
  return if( ! $LOG );

  my $elapsed_time = sprintf("%.2f", Time::HiRes::tv_interval($start_time)) if( $start_time );
  my @line = ();
  push(@line, $ENV{'REMOTE_HOST'} || '-',
              get_iso_date(), 
	      $query->param('q') || '-',
              scalar(keys %answer),
	      $query->param('p') || 1,
	      $elapsed_time || '-');
  
  use Fcntl ':flock';		# import LOCK_* constants
  open(LOG, ">>$LOGFILE") or die "Cannot open logfile '$LOGFILE' for writing: $!";
  flock(LOG, LOCK_EX);
  seek(LOG, 0, 2);
  print LOG join(';', @line).";\n";
  flock(LOG, LOCK_UN);
  close(LOG);
}

# Return current date and time in ISO 8601 format, i.e. yyyy-mm-dd hh:mm:ss
sub get_iso_date {
  use Time::localtime;
  my $date = (localtime->year() + 1900).'-'.two_digit(localtime->mon() + 1).'-'.two_digit(localtime->mday());
  my $time = two_digit(localtime->hour()).':'.two_digit(localtime->min()).':'.two_digit(localtime->sec());
  return "$date $time";
}

# Returns "0x" for "x" if x is only one digit, otherwise it returns x unmodified.
sub two_digit {
  my $value = $_[0];
  $value = '0'.$value if( length($value) == 1 );
  return $value;  
}
