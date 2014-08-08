#!/usr/bin/perl


if (scalar(@ARGV) != 2) {
    &usage;
    exit;
}

&header;

# remove a trailing /
$dirpath = $ARGV[0];
$dirpath =~ s!(.*)/$!$1!;

$dirpath2 = $ARGV[1];
$dirpath2 =~ s!(.*)/$!$1!;

$max_uid = -1;

opendir FILEDIR, $dirpath or die "Error opening filedir!\n";

while ($filename = readdir FILEDIR) {
    if ($filename =~ m/^\./) { next; } #ignore . and ..

    if ( ! (($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
    $atime,$mtime,$ctime,$blksize,$blocks) = stat $dirpath ."/". $filename )) {
        print "Error stat-ing the file $pathdir/$filename!\n" ; next; }
    
    $type = ($mode & 070000) >> 12;
    
    if ($type != 0) {
	print "File ". $dirpath ."/". $filename ." its not regular\n";
	next;
    }
    convertfile($filename);
}

closedir FILEDIR;

sub convertfile {
    my $filen = shift;
    my ($tab, $col, $val);

    open FILE, $dirpath ."/". $filen or die "Error opening file ". $dirpath ."/". $filen ."\n";
    if (open FILEOUT, $dirpath2 ."/". $filen) {
	die "Found existent file : ". $dirpath2 ."/". $filen ."!! Plese provide an empty output directory !!";
    }

    open FILEOUT, ">". $dirpath2 ."/". $filen or die "Error opening output file ". $dirpath2 ."/". $filen ."\n";
    print "Converting file ". $filen ."... ";

    while($line = <FILE>) {
	if ($line =~ m/".*"=".*"/) {

	    $line =~ m/^"([A-Za-z0-9]+)\\\\(.*)"="(.*)"\n$/;

	    $tab = $1;
	    $col = $2;
	    $val = $3;

	    if ($tab =~ m/^team$/i) {
		print FILEOUT "\"". $tab . "\\\\WAR3\\\\" . $col . "\"=\"" . $val ."\"\n";
		next;
	    }

	    if ($tab =~ m/^record$/i) {
		if ($col =~ m/^solo/i or $col =~ m/^team/i or $col =~ m/^ffa/i or
		    $col =~ m/^orcs/i or $col =~ m/^humans/i or $col =~ m/^nightelves/i or $col =~ m/^undead/i or $col =~ m/^random/i) {
		    print FILEOUT "\"". $tab . "\\\\WAR3\\\\" . $col . "\"=\"" . $val ."\"\n";
		    next;
		}
	    }

	    print FILEOUT $line;
	}
    }
    close FILEOUT;
    close FILE;
    print "done\n";
}

sub header {
    print "Account files WAR3 converting tool.\n";
    print " Copyright (C) 2003 Dizzy (dizzy\@roedu.net)\n";
    print " People Versus People Gaming Network (www.pvpgn.org)\n\n";
}

sub usage {
    &header;
    print "Usage:\n\n\tconvert_w3.pl <inputdir> <outputdir>\n\n";
    print "\t <inputdir>\t: directory with the input account files\n";
    print "\t <outputdir>\t: directory where the output account files will reside\n";
}
