use strict;
use IO::All;
use LWP::UserAgent;
use URI;

my $bearer_token = $ARGV[0];
my $file_name = $ARGV[1];

print "reading file ", $ARGV[1], "\n";
my $text = io($ARGV[1])->utf8->slurp;
$text = "[TOC]\n\n" . $text;

my $uri = URI->new('https://sourceforge.net/rest/p/log4cplus/wiki/');
$uri->path($uri->path . $ARGV[1]);
$uri->query_form('access_token' => $bearer_token);

print "POSTing to ", $uri->as_string, "\n";
my $ua = LWP::UserAgent->new;
push @{ $ua->requests_redirectable }, 'POST';

my $req = HTTP::Request->new(POST => $uri);
$req->content_type("application/x-www-form-urlencoded; charset='utf8'");
#$req->content(Encode::encode_utf8($utf8));
$req->accept_decodable;
my $content_enc = URI->new;
$content_enc->query_form('text' => $text, 'labels' => 'readme');
$req->content($content_enc->query);
print "POST body:\n", $req->as_string, "\n";

my $resp = $ua->request($req);
print "POST response: ";
if ($resp->is_success)
{
    print $resp->decoded_content, "\n";
}
else
{
    print $resp->decoded_content, "\n";
    die $resp->status_line;
}
