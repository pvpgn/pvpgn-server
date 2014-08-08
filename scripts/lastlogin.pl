#!/usr/bin/perl

#Usage: lastlogins.pl <user directory>

$ARGV[1] eq "" && die "error arguments"; 

$userdir=$ARGV[0]; 
opendir(DIR,$userdir) || die "error open dir $userdir"; 
@files= grep { /^[^.]/ && -f "$userdir/$_" } readdir(DIR); 
closedir DIR; 

$tbl{'name'}='"BNET\\acct\\lastlogin_time"'; 
foreach (@files) { 
        open(S_FILE,"$s_dir/$_") || die "error open s_file"; 
        $dest_file=lc("$d_dir/$_"); 
        while (<S_FILE>) { 
                chop($_); 
                ($name,$value)=split(/:/,$_); 
                foreach (keys %tbl) { 
                        if ($_ eq $name) { 
                                if ( $_ eq "password" ) { 
                                         $value=&passconv($value); 
                                } 
                                break; 
                        } 
                } 
        } 
        $userid++; 
        close(S_FILE); 
} 
