#!/usr/local/bin/perl
# example template from: https://www.tohoho-web.com/wwwcgi4.htm


#
# escape for HTML words
#
sub html {
    $str = $_[0];
    $str =~ s/&/&amp;/g;
    $str =~ s/</&lt;/g;
    $str =~ s/>/&gt;/g;
    $str =~ s/"/&quot;/g;
    $str =~ s/'/&#x27;/g;
    return $str;
}    

#
# write header
#
print <<EOF;
Content-Type: text/html

<!doctype html>
<html>
<head>
<meta charset="utf-8">
<title>CGI TEST</title>
</head>
<body>
<h1>CGI TEST</h1>
<pre>
EOF

#
# argument
#
print "=================================\n";
print "argument\n";
print "=================================\n";
for ($i = 0; $i <= $#ARGV; $i++) {
	print "ARGV[$i] = [ " . html($ARGV[$i]) . " ]\n";
}
print "\n";

#
# environement for CGI
#
print "=================================\n";
print "environement\n";
print "=================================\n";
print "AUTH_TYPE = [ " . html($ENV{'AUTH_TYPE'}) . " ]\n";
print "CONTENT_LENGTH = [ " . html($ENV{'CONTENT_LENGTH'}) . " ]\n";
print "CONTENT_TYPE = [ " . html($ENV{'CONTENT_TYPE'}) . " ]\n";
print "GATEWAY_INTERFACE = [ " . html($ENV{'GATEWAY_INTERFACE'}) . " ]\n";
print "HTTP_ACCEPT = [ " . html($ENV{'HTTP_ACCEPT'}) . " ]\n";
print "HTTP_FORWARDED = [ " . html($ENV{'HTTP_FORWARDED'}) . " ]\n";
print "HTTP_REFERER = [ " . html($ENV{'HTTP_REFERER'}) . " ]\n";
print "HTTP_USER_AGENT = [ " . html($ENV{'HTTP_USER_AGENT'}) . " ]\n";
print "HTTP_X_FORWARDED_FOR = [ " . html($ENV{'HTTP_X_FORWARDED_FOR'}) . " ]\n";
print "PATH_INFO = [ " . html($ENV{'PATH_INFO'}) . " ]\n";
print "PATH_TRANSLATED = [ " . html($ENV{'PATH_TRANSLATED'}) . " ]\n";
print "QUERY_STRING = [ " . html($ENV{'QUERY_STRING'}) . " ]\n";
print "REMOTE_ADDR = [ " . html($ENV{'REMOTE_ADDR'}) . " ]\n";
print "REMOTE_HOST = [ " . html($ENV{'REMOTE_HOST'}) . " ]\n";
print "REMOTE_IDENT = [ " . html($ENV{'REMOTE_IDENT'}) . " ]\n";
print "REMOTE_USER = [ " . html($ENV{'REMOTE_USER'}) . " ]\n";
print "REQUEST_METHOD = [ " . html($ENV{'REQUEST_METHOD'}) . " ]\n";
print "SCRIPT_NAME = [ " . html($ENV{'SCRIPT_NAME'}) . " ]\n";
print "SERVER_NAME = [ " . html($ENV{'SERVER_NAME'}) . " ]\n";
print "SERVER_PORT = [ " . html($ENV{'SERVER_PORT'}) . " ]\n";
print "SERVER_PROTOCOL = [ " . html($ENV{'SERVER_PROTOCOL'}) . " ]\n";
print "SERVER_SOFTWARE = [ " . html($ENV{'SERVER_SOFTWARE'}) . " ]\n";
print "\n";

#
# write env to form
#
print "=================================\n";
print "witch form?\n";
print "=================================\n";
if ($ENV{'REQUEST_METHOD'} eq "POST") {
	# If it's POST read by STDIN
	read(STDIN, $query_string, $ENV{'CONTENT_LENGTH'});
} else {
	# If it's GET read by enironement variable 
	$query_string = $ENV{'QUERY_STRING'};
}
# "ariable1=value1&variable2=value2" split by &
@a = split(/&/, $query_string);
foreach $a (@a) {
	# split by =
	($name, $value) = split(/=/, $a);
	# decode + and  %8A 
	$value =~ tr/+/ /;
	$value =~ s/%([0-9a-fA-F][0-9a-fA-F])/pack("C", hex($1))/eg;
	# write variable and value 
	print "$name = [ " . html($value) . " \n";
	# insert in $FORM{'value'} if we use after
	$FORM{$name} = $value;
}

#
# write footer
#
print <<EOF;
</pre>
</body>
</html>
EOF