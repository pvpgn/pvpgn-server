#!/usr/bin/perl

$| = 1;

use DBI;

&header;

if (scalar(@ARGV) < 5) {
	&usage;
	exit;
}

$levelxpfile = shift;
$dbhost = shift;
$dbname = shift;
$dbuser = shift;
$dbpass = shift;

open LEVELFD, $levelxpfile or die "Could not open XP level file ($levelxpfile)";

print "Reading from XP file ...\n";
while($line = <LEVELFD>) {
	$line =~ s/#.*//;
	if ($line !~ m/^\s*$/) {
		($level, $startxp, $neededxp, $lossfactor, $mingames) = split(/\s+/,$line);
		print "level: $level startxp: $startxp neededxp: $neededxp lossfactor: $lossfactor mingames: $mingames\n";
		$xplevel[$level]->{startxp} = $startxp;
		$xplevel[$level]->{neededxp} = $neededxp;
		$xplevel[$level]->{lossfactor} = $lossfactor;
		$xplevel[$level]->{mingames} = $mingames;
	}
}

close LEVELFD;
print "...done\n\nConnecting to db...";

$dbh = DBI->connect("dbi:mysql:$dbname;host=$dbhost", $dbuser, $dbpass) or die "Could not connect to db (mysql, db: $dbname, host: $dbhost, user: $dbuser, pass: $dbpass)";
$sth = $dbh->prepare("SELECT uid, solo_xp, solo_level FROM Record WHERE uid > 0") or die "Error preparing query!";

$sth->execute() or die "Error executing query!";

while($row = $sth->fetchrow_hashref) {
	$level = $row->{solo_level};
	$xp = $row->{solo_xp};
	if ($level eq "" or $xp eq "" or $level < 1) { next; }
	for($i = $level; $i < 50 ; $i++) {
		if ($xplevel[$i]->{startxp} > $xp) { $i--;  last; }
	}
	$level = $i;
	
	for($i = $level; $i > 1; $i++) {
		if ($xplevel[$i]->{needed_xp} <= $xp) { last; }
	}
	$level = $i;
	
	if ($level < 1) {$level = 1;}
	
	if ($level != $row->{solo_level}) {
		print "Repaired $uid from (level: " , $row->{solo_level}, " xp: $xp) to (level: $level xp: $xp)\n";
		$dbh->do("UPDATE profile SET solo_level = '$level' WHERE uid = '".$row->{uid}."'");

	}

}

$sth->finish();
$dbh->disconnect();

sub header {
    print "Account db accounts XP levels repair tool.\n";
    print " Copyright (C) 2002 Dizzy (dizzy\@roedu.net)\n";
    print " People Versus People Gaming Network (www.pvpgn.org)\n\n";
}

sub usage {
    print "Usage:\n\nrepairlevels.pl <xplevelfile> <dbhost> <dbname> <dbuser> <dbpass>\n\n";
    print " <xplevelfile>\t: path/name of the file with the XP level data (ex. bnxplevel.txt)\n";
    print " <dbhost>\t: the database host\n";
    print " <dbname>\t: the database name\n";
    print " <dbuser>\t: the database user\n";
    print " <dbpass>\t: the database password\n";
}
