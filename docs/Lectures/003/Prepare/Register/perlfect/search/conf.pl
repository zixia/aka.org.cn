# Perlfect Search Configuration file

# NOTE: Whenever you change one of the options that's marked with [re-index]
# you need to run indexer.pl again to make the change take effect.

###########################################################################
### basic configuration
### You'll have to adapt these values if you didn't use setup.pl

# Where you want the indexer to start. [re-index]
$DOCUMENT_ROOT = '/home/akaWWW/public_html/';

# The base url of your site.
$BASE_URL = 'http://aka.org.cn';

# The url in which Perlfect Search is located (usually somewhere in cgi-bin/).
$CGIBIN = 'http://aka.org.cn/cgi-bin/perlfect/search/';

# The full-path of the directory where Perlfect Search is installed.
$INSTALL_DIR = '/home/akaWWW/cgi-bin/perlfect/search/';

# Only files with these extensions should be indexed. [re-index]
@EXT = ("htm","html","shtml","txt");

###########################################################################
### http configuration
### You only need this if you want to index your pages via http

# Where you want the indexer to start via http. Leave empty if
# you want to index the files in the filesystem ($DOCUMENT_ROOT).
# WARNING: do not use foreign servers, the scripts might run out
# of control and try to fetch an infinite number of pages. [re-index]
# example: $HTTP_START_URL = 'http://localhost/';
$HTTP_START_URL = 'http://aka.org.cn';

# The web server's document root. Normally that's the same as $DOCUMENT_ROOT,
# it differs if you're only using Perlfect Search on a subdirectory [re-index].
$HTTP_SERVER_ROOT = $DOCUMENT_ROOT;

# Only if indexing via http: limit crawling to this URL. This is an
# important setting so the script doesn't run out of control. [re-index]
$HTTP_LIMIT_URL = $HTTP_START_URL;

# Only if indexing via http: the content types to index. [re-index]
@HTTP_CONTENT_TYPES = ('text/html', 'text/plain');

# Set to 1 to get verbose output during indexing.
$HTTP_DEBUG = 1;

###########################################################################
### advanced configuration
### You only need this if you want to adapt advanced features

# Program that converts PDF to ascii text. pdftotext is part of xpdf, available
# at http://www.foolabs.com/xpdf/download.html. You also have to add "pdf" 
# to @EXT and your PDF files must have a ".pdf" suffix. You can use any program
# that will print ASCII to STDOUT if called this way: "program pdf_filename -". 
# WARNING: The PDFs filenames may not include special characters for security 
# reasons, still it is recommended to use this option only to index your own 
# files, not other people's files which filenames you cannot control. [re-index]
$PDFTOTEXT = '/usr/bin/pdftotext';

# How many results should be shown per page.
$RESULTS_PER_PAGE = 5;

# Show the ranking in percent, with the first document = 100%.
$PERCENTAGE_RANKING = 1;

# Do you want to index numbers? If so set $INDEX_NUMBERS to 1. [re-index]
$INDEX_NUMBERS = 0;

# If you don't have enough memory, set this to 1. This will slow down 
# indexer.pl by a factor of about 2. Searching is not affected.
$LOW_MEMORY_INDEX = 0;

# How much of the document should be put in the index? With this option,
# the context of the match is shown on the results page. This only works
# if the match was in the first $CONTEXT_SIZE bytes of the document.
# Warning: Using this option will generate a very big index file.
# Set to 0 to disable, set to -1 for no limit. [re-index]
$CONTEXT_SIZE = 0;

# If $CONTEXT_SIZE is enabled, how many occurences of every term should be shown
# on the results page?
$CONTEXT_EXAMPLES = 2;

# If $CONTEXT_SIZE is enabled, how many words should be used to show the context
# of a term?
$CONTEXT_DESC_WORDS = 12;

# How many words should be used from the <BODY> of an html document as a 
# description for the document in case there is no <META description> tag 
# available and $CONTEXT_SIZE is 0. [re-index]
$DESC_WORDS = 25;

# The minimum length of a word. Any word of smaller size is not indexed. 
# [re-index]
$MINLENGTH = 3;

# If you have umlauts or accents etc. in your text, enable this.
# With this option accented characters will be indexed as the characters
# they are based on (e.g. è -> e, ü -> u), without this option they will
# be filtered out completely (you don't want that). [re-index]
$SPECIAL_CHARACTERS = 1;

# The largest acceptable word size. Reducing this saves space but decreases
# result accuracy. Setting the variable to 0 ignores stemming alltogether and 
# also makes the indexer a bit faster. [re-index]
$STEMCHARS = 0;

# Add URLs to the index, so one can search for them? Note that special
# characters will be ignored, just as in normal text. [re-index]
$INDEX_URLS = 0;

# You can completely ignore certain parts of your documents if you put these 
# HTML comments around them. [re-index]
$IGNORE_TEXT_START = '<!--ignore_perlfect_search-->';
$IGNORE_TEXT_END = '<!--/ignore_perlfect_search-->';

# How much more important are words found in the title, in the meta values
# (author, description, keywords), and in the headlines compared to normal 
# text in the body? This influences the ranking of the results.
# Use any integer (0 = ignore that text completely) [re-index]
$TITLE_WEIGHT = 5;
$META_WEIGHT = 5;
$H_WEIGHT{'1'} = 5;	# headline <h1>...</h1>
$H_WEIGHT{'2'} = 4;
$H_WEIGHT{'3'} = 3;
$H_WEIGHT{'4'} = 1;
$H_WEIGHT{'5'} = 1;
$H_WEIGHT{'6'} = 1;	# headline <h6>...</h6>

# If you want to log the queries to an extra file, set this to 1.
# Every use of search.pl will then be logged to data/log.txt. That file
# has to exist and must be writable for the webserver. The line format is:
# REMOTE_HOST;date;terms;matches;current page;(time to search in seconds);
# NOTE: if you have many queries, this file will grow quite fast.
$LOG = 0;

# This will increase the score of results that contain more than one of
# the searched terms. Queries with only one term will not be affected.
# The number given here is a factor that multiplies the score (even
# several times, if there are more than two terms). 0 turns it off.
$MULTIPLE_MATCH_BOOST = 0;

# Directory with templates (normally you don't have to modify this).
$TEMPLATE_DIR = $INSTALL_DIR.'templates/';

# What's the default language. This is the language that's used if no lang
# parameter is passed to the script or if the parameter is invalid.
$DEFAULT_LANG = 'en';

# The result template for several languages.
$SEARCH_TEMPLATE{'en'} = $TEMPLATE_DIR.'search.html';
$SEARCH_TEMPLATE{'de'} = $TEMPLATE_DIR.'search_de.html';
$NO_MATCH_TEMPLATE{'en'} = $TEMPLATE_DIR.'no_match.html';
$NO_MATCH_TEMPLATE{'de'} = $TEMPLATE_DIR.'no_match_de.html';

# The text for the "Next Page" link in several languages.
$NEXT_PAGE{'en'} = 'Next';
$NEXT_PAGE{'de'} = 'n&auml;chste Seite';

# The text for the "Previous Page" link in several languages.
$PREV_PAGE{'en'} = 'Previous';
$PREV_PAGE{'de'} = 'vorige Seite';

###########################################################################
### You shouldn't have to edit anything below this line.

# Various paths (do NOT use system-wide /tmp for security reasons!)
$TMP_DIR  = $INSTALL_DIR.'temp/';
$DATA_DIR = $INSTALL_DIR.'data/';
$CONF_DIR = $INSTALL_DIR."conf/";
$STOPWORDS_FILE = $CONF_DIR.'stopwords.txt';
$NO_INDEX_FILE = $CONF_DIR.'no_index.txt';
$LOGFILE = $DATA_DIR.'log.txt';
$SEARCH = 'search.pl';
$SEARCH_URL = $CGIBIN.$SEARCH;

# Paths to the database files.
$INV_INDEX_DB_FILE = $DATA_DIR.'inv_index';
$DOCS_DB_FILE      = $DATA_DIR.'docs';
$SIZES_DB_FILE     = $DATA_DIR.'sizes';
$TERMS_DB_FILE     = $DATA_DIR.'terms';
$DF_DB_FILE        = $DATA_DIR.'df';
$TF_DB_FILE        = $DATA_DIR.'tf';
$CONTENT_DB_FILE   = $DATA_DIR.'content';
$DESC_DB_FILE      = $DATA_DIR.'desc';
$TITLES_DB_FILE    = $DATA_DIR.'titles';

# Paths to the temporary database files.
$INV_INDEX_TMP_DB_FILE = $DATA_DIR.'inv_index_tmp';
$DOCS_TMP_DB_FILE      = $DATA_DIR.'docs_tmp';
$SIZES_TMP_DB_FILE     = $DATA_DIR.'sizes_tmp';
$TERMS_TMP_DB_FILE     = $DATA_DIR.'terms_tmp';
$CONTENT_TMP_DB_FILE   = $DATA_DIR.'content_tmp';
$DESC_TMP_DB_FILE      = $DATA_DIR.'desc_tmp';
$TITLES_TMP_DB_FILE    = $DATA_DIR.'titles_tmp';

# Official version number.
$VERSION = "3.20";
1;
