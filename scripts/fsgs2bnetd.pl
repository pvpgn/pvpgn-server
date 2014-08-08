#!/usr/bin/perl

#Usage: fsgs2bnetd "source directory" "destination directory"
#source directory:    directory contains fsgs account files 
#destnation directory: directory to save newly created bnetd 
#                      accounts,should exist before run this 
# 
#there is another change made to bnetd account in this script 
#i think fsgs account filename format <username> is better 
#than using bnetd's <userid>, so i kept it in fsgs format. 
#you may change it by edit the script yourself 
  
$ARGV[1] eq "" && die "error arguments"; 

$s_dir=$ARGV[0]; 
$d_dir=$ARGV[1]; 

opendir(DIR,$s_dir) || die "error open dir"; 
@files= grep { /^[^.]/ && -f "$s_dir/$_" } readdir(DIR); 
closedir DIR; 

$userid=1; 
$fsgs{'name'}="\"BNET\\\\acct\\\\username\""; 
$fsgs{'password'}="\"BNET\\\\acct\\\\passhash1\""; 

foreach (@files)
{
        open(S_FILE,"$s_dir/$_") || die "error open s_file: $_\n";

	$dest_file=lc("$d_dir/$_");	
        open(D_FILE,">$dest_file") || die "error open d_file: $_\n";

	while (<S_FILE>)
	{ 
                chop($_); 
                ($name,$value)=split(/:/,$_); 

		foreach (keys %fsgs)
		{ 
                        if ($_ eq $name)
			{ 
                                if ( $_ eq "password" )
				{
                                         $value=&passconv($value);
                                }
                                print D_FILE "$fsgs{$_}=\"$value\"\n"; 
                                break;
                        }
                }
        }
	
        print D_FILE "\"BNET\\\\acct\\\\userid\"=\"$userid\"\n"; 
        $userid++; 
        close(S_FILE); 
        close(D_FILE); 
}

sub passconv
{
	($f_pass)=@_;
	my ($d_pass)="";
	$f_pass=lc($f_pass);
	$length=length($f_pass);
	
	for ($i=0;$i<=$length;$i=$i+2)
	{
	        $a=2*(int(($i/2)/4)*8+3-$i/2);
        	$d_pass .= substr($f_pass,$a,2);
	}
	
	return $d_pass;
}
