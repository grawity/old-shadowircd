#!/usr/bin/perl
# Configure frontend for Shadow.
# $Id: config.pl,v 1.1 2004/09/07 04:50:23 nenolod Exp $

print "Welcome to the configuration wizard for ShadowIRCd.\n";

print "Where do you wish for the ircd to be installed?\n";
$prefix = "/home/ircd/shadow";
print "[$prefix] ";
$prefix = <STDIN>;
chomp $prefix;
$prefix = "/home/ircd/shadow" if (!$prefix);

print "\n";

print "Checking what operating system you are running... ";
$os = $^O;
print "$os\n";

print "\n";

print "Which I/O system do you want to use?\n";
print "rtsigio............. Great on Linux.\n" if ($os eq "linux");
print "kqueue.............. Great on FreeBSD.\n" if ($os eq "freebsd");
print "devpoll............. Great on Solaris.\n" if ($os eq "sunos");
print "poll................ No issues.\n";
print "select.............. No issues.\n";
print "[poll] ";
$ioeng = <STDIN>;
chomp $ioeng;
$ioeng = "poll" if (!$ioeng);

print "\n";
print "Do you want to enable OpenSSL support? May have issues on Linux if\n";
print "using rtsigio as the I/O system.\n";
print "[no] ";
$openssl = <STDIN>;
chomp $openssl;
$openssl = "no" if (!$openssl);

print "\n";
print "Do you want to disable Module support? This may be helpful\n";
print "if you are experiencing issues with stability.\n";
print "[no] ";
$dismod = <STDIN>;
chomp $dismod;
$dismod = "no" if (!$dismod);

print "\n";

print "Do you want to define custom NICKLEN and TOPICLEN values?\n";
print "[no] ";
$nicklen = 20;
$topiclen = 450;
$editval = <STDIN>;
chomp $editval;

if ($editval eq "yes") {
  print "\n";
  print "What value do you want for your NICKLEN?\n";
  print "[$nicklen] ";
  $nicklen = <STDIN>;
  chomp $nicklen;
  $nicklen = 20 if (!$nicklen);

  print "\n";

  print "What value do you want for your TOPICLEN?\n";
  print "[$topiclen] ";
  $topiclen = <STDIN>;
  chomp $topiclen;
  $topiclen = 450 if (!$topiclen);

}

print "\n";

print "Do you have a network under 2,000 users and therefore want to enable\n";
print "optimizations for small networks?\n";
$smallnet = "no";
print "[$smallnet] ";
$smallnet = <STDIN>;
chomp $smallnet;
$smallnet = "no" if (!$smallnet);

print "\n";

print "Would you like to disable chanowner (+u, ! prefix)?\n";
$nochown = "no";
print "[$nochown] ";
$nochown = <STDIN>;
chomp $nochown;
$nochown = "no" if (!$nochown);

print "\n";

print "Do you want to enable assertions? (for debug builds)\n";
$assert = "no";
print "[$assert] ";
$assert = <STDIN>;
chomp $assert;
$assert = "no" if (!smallnet);

print "\n";

print "Building arguments to ./configure script... ";

$configure = "./configure ";
$configure .= "--prefix=" . $prefix;
$configure .= " --enable-$ioeng";
$configure .= " --enable-openssl" if ($openssl eq "yes");
$configure .= " --with-nicklen=$nicklen --with-topiclen=$topiclen";
$configure .= " --enable-small-net" if ($smallnet eq "yes");
$configure .= " --enable-assert" if ($assert eq "yes");
$configure .= " --disable-assert" if ($assert eq "no");
$configure .= " --disable-shared-modules" if ($dismod eq "yes");
$configure .= " --enable-no-chan-owner" if ($nochown eq "yes");

print "done.\n";

print "$configure\n";
exec $configure;

