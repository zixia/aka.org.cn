# Extension to Perlfect Search to allow fetching HTML files via http
# Daniel Naber <daniel.naber@t-online.de>, 2000-05-08, last update 2000-10-26

use LWP::UserAgent;
use URI;
use MD5;

my $md5 = new MD5;
my $http_user_agent = LWP::UserAgent->new;
my $host = "";
my $base = "";
my %list;
my $cloned_documents;
my @no_index;

## Get the host and base part of $BASE_URL
sub init_http {
	my $uri = new URI($BASE_URL);
	$host = $uri->host;
	$base = $uri->scheme.":".$uri->opaque;
	$cloned_documents = 0;
}

## Fetch $url and all URLs that this document links to. Remember
## visited documents and their checksums in %list
sub crawl_http {
	my $url = shift;
	# fetch URL via http, if not yet visited:
	foreach $visited_url (values %list) {
		if( $url eq $visited_url ) {
			print "Ignoring '$url': already visited\n" if ( $HTTP_DEBUG );
			return;
		}
	}
	my ($url, $content) = get_url($url);
	$content = parse_pdf($content, $url);
	# Calculate checksum of content:
	$md5->reset();
	$md5->add($content);
	my $digest = $md5->hexdigest();
	# URL with the same content already visited?:
	if( $list{$digest} ) {
		print "Ignoring '$url': content identical to '$list{$digest}'\n" if ( $HTTP_DEBUG );
		$list{'clone_'.$cloned_documents} = $url;
		$cloned_documents++;
		return;
	}
	# return if content could not be fetched, but before remember digest and URL:
	$list{$digest} = $url;
	return if( ! $url );
	# Perlfect Search expects filenames, so make one:
	my $filename = filename_from_url($url);
	# call the Perlfect Search index functions:
	my $doc_id = record_file($filename);
	index_file($url, $doc_id, $content);
	# 'parse' HTML for new URLs (Meta-Redirects and Anchors):
	while( $content =~ m/
			content\s*=\s*["'][0-9]+;\s*URL\s*=\s*(.*?)['"]
			|
			href\s*=\s*["'](.*?)['"]
			/gisx ) {
		my $new_url = $+;
		# &amp; in a link to distinguish arguments is actually correct, but we have to
		# convert it to fetch the file:
		$new_url =~ s/&amp;/&/g;
		my $next_url = next_url($url, $new_url);
		if ( $next_url ) {
			crawl_http($next_url);
		}
	}

}

## Return an absolute version of the $new_url, which is relative
## to $url. If URL is not accepted or has been visited already,
## return nothing
sub next_url {
	my $base_url = shift;
	my $new_url = shift;
	my $new_uri = URI->new_abs($new_url, $base_url);
	# get rid of "#fragment":
	$new_uri = URI->new($new_uri->scheme.":".$new_uri->opaque);
	# get the right URL even if the link has too many "../":
	my $path = $new_uri->path;
	$path =~ s/\.\.\///g;
	$new_uri->path($path);
	$new_url = $new_uri->as_string;
	if( check_accept_url($new_url) ) {
		return $new_url;
	} else {
		return;
	}
}

## Check if URL is accepted, return 1 if yes, 0 otherwise
sub check_accept_url {
	my $url = shift;
	my $reject;
	# ignore "empty" links (shouldn't happen):
	if( ! $url || $url eq '' ) {
		$reject = "empty/undefined URL";
	}
	# ignore foreign servers/URLs and non-http protocols:
	if( $url =~ m/:\/\// && $url !~ m/^$HTTP_LIMIT_URL/i ) {
		$reject = "not below \$HTTP_LIMIT_URL or non-http protocol";
	}
	# ignore file links:
	if( $url =~ m/^file:/i ) {
		$reject = "file URL";
	}
	# ignore javascript: links:
	if( $url =~ m/^javascript:/i ) {
		$reject = "javascript link";
	}
	# ignore mailto: links:
	if( $url =~ m/^mailto:/i ) {
		$reject = "mailto link";
	}
	# ignore document internal links:
	if( $url =~ m/^#/i ) {
		$reject = "local link";
	}
	my $ignore_reason = to_be_ignored($url);
	if( $ignore_reason ) {
		$reject = $ignore_reason;
	}
	if( $reject ) {
		print "Ignoring '$url': $reject\n" if( $HTTP_DEBUG );
		return 0;
	}
	return 1;
}

## Fetch URL via http, return real URL (differs only in case of redirect) and
## document's contents. Return nothing in case of error or unwanted Content-Type
sub get_url {
	my $url = shift;
	my $request = HTTP::Request->new(GET => $url);
	my $response = $http_user_agent->request($request);
	my $buffer = $response->content;
	my ($content_type) = ($response->headers_as_string =~ m/^Content-Type:\s*(.+)$/im);
	$content_type =~ s/^(.*?);.*$/$1/;		# ignore possible charset value
	if( ! grep(/$content_type/i, @HTTP_CONTENT_TYPES) ) {
 		print STDERR "Ignoring '$url': content-type '$content_type'\n" if( $HTTP_DEBUG );
		return;
	}
	if( $response->is_error ) {
 		print STDERR "Error: Couldn't get '$url': response code " .$response->code. "\n";
		return;
	}
	my $size = length($buffer);
	print "Fetched  '$url', $size bytes\n" if( $HTTP_DEBUG );
	# maybe we are redirected, so use the new URL, but avoid possible 
	# change of hostname (e.g. "localhost" -> "foo.bar.org"):
	$uri = URI->new_abs($response->base, $BASE_URL);
	if( $uri->host ne $host ) {
		$uri->host($host);
	}
	return ($uri->as_string, $buffer);
}

## Make a filename by removing "http://<hostname>/" and "#fragment" from URL
## and prepending $DOCUMENT_ROOT
sub filename_from_url {
	my $url = shift;
	my $uri = new URI($url);
	my $filename = $uri->path;
	$filename .= "?".$uri->query if( $uri->query );
	$filename =~ s/^\/// if( $HTTP_SERVER_ROOT =~ m/\/$/ );
	$filename = $HTTP_SERVER_ROOT  . $filename;
	return $filename;
}

1;
