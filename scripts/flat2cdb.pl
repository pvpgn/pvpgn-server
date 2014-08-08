#!/usr/bin/perl


$cdb_command = "/usr/local/bin/cdb";

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
    my ($key, $val, $outpath);

    open FILE, $dirpath ."/". $filen or die "Error opening file ". $dirpath ."/". $filen ."\n";
    $outpath = $dirpath2 ."/". $filen;
    if (open FILEOUT, $outpath) {
	die "Found existent file : ". $outpath ."!! Plese provide an empty output directory !!";
    }

    close FILEOUT;
    open CDB, "| $cdb_command -c -t '". $outpath . ".tmp' '". $outpath ."'" || die "Error running $cdb_command\n";
    print "Converting file ". $filen ."... ";

    while($line = <FILE>) {
	if ($line =~ m/^#/) { next; }
	if ($line =~ m/^"(.*)"="(.*)"\s*$/) {

	    $key = $1;
	    $val = $2;

	    $key =~ s/\\\\/\\/g;
	    print CDB "+". length($key) .",". length($val) .":". $key ."->". $val ."\n";
	}
    }
    print CDB "\n";
    close CDB;
    close FILE;
    print "done\n";
}

sub header {
    print "Account files flat2cdb converting tool.\n";
    print " Copyright (C) 2003 Dizzy (dizzy\@roedu.net)\n";
    print " People Versus People Gaming Network (www.pvpgn.org)\n\n";
}

sub usage {
    &header;
    print "Usage:\n\n\tflat2cdb.pl <inputdir> <outputdir>\n\n";
    print "\t <inputdir>\t: directory with the input flat account files\n";
    print "\t <outputdir>\t: directory where the output cdb account files will reside\n";
}
