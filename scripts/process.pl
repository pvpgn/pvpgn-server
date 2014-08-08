#!/usr/bin/perl -w
#!/usr/local/bin/perl -w
#!/opt/perl/bin/perl -w
#
# Copyright (C) 1999  Mark Baysinger (mbaysing@ucsd.edu)
# Copyright (C) 1999,2000,2001  Ross Combs (rocombs@cs.nmsu.edu)
# Copyright (C) 2001  Roland Haeder (webmaster@ai-project-now.de)
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

# Version of this script
$version     = "v1.05";

# modify to use correct locations
$infile  = "./bnetdlist.txt";
#$infile  = "/var/tmp/bnetdlist.txt";
$outfile = "./bnetdlist.html";
#$outfile = "/home/httpd/html/bnetdlist.html";
$tmpfile = $outfile . ".temp";

# document look
$page_bg_color     = "#202020";
$link_color        = "#5070FF";
$alink_color       = "#FF3030";
$vlink_color       = "#8090FF";
$tbl_border_color  = "#404040";
$tbl_border_ramp   = "0";
$tbl_border_width  = "2";
$tbl_padding       = "1";
$tbl_head_bg_color = "$tbl_border_color";
$tbl_bg_color      = "#000000";
$tbl_text_color    = "#80A0FF";
$title_bg_color    = "$page_bg_color";
$title_text_color  = "#FFFFFF";
$sum_bg_color      = "$page_bg_color";
$sum_text_color    = "$title_text_color";

# header strings
$title = "BNETD Server Status";
$header = "<br>$title<br>";



open(INFILE, "<$infile") || die("Can't open \"$infile\" for reading: $!\n");
$/ = "\#\#\#\n";
@servers = <INFILE>;
chomp @servers;
close INFILE;


open(OUTFILE, ">$tmpfile") || die("Can't open \"$tmpfile\" for writing: $!\n");

print OUTFILE "<html>\n";
print OUTFILE "<head>\n";
print OUTFILE "  <meta name=\"GENERATOR\" content=\"process.pl $version\">\n";
print OUTFILE "  <title>$title</title>\n";
print OUTFILE "</head>\n";
print OUTFILE "<body bgcolor=\"$page_bg_color\" text=\"$tbl_text_color\" link=\"$link_color\" vlink=\"$vlink_color\" alink=\"$alink_color\">\n";
print OUTFILE "  <font face=\"Verdana,Helvetica,Arial\">\n";

print OUTFILE "    <table border=\"0\" width=\"100%\">\n";
print OUTFILE "      <tr>\n";
print OUTFILE "        <td align=middle bgcolor=\"$title_bg_color\"><font color=\"$title_text_color\">$header</font><br></td>\n";
print OUTFILE "      </tr>\n";
print OUTFILE "      <tr>\n";
print OUTFILE "        <td align=middle>\n";
print OUTFILE "          <table border=\"0\" cellspacing=\"0\" bgcolor=\"$tbl_border_color\">\n";
print OUTFILE "            <tr>\n";
print OUTFILE "              <td>\n";
print OUTFILE "                <table border=\"$tbl_border_ramp\" cellspacing=\"$tbl_border_width\" cellpadding=\"$tbl_padding\" bgcolor=\"$tbl_bg_color\">\n";
print OUTFILE "                  <tr>\n";
print OUTFILE "                    <th bgcolor=\"$tbl_head_bg_color\"><font size=2>Address</font></th>\n";
print OUTFILE "                    <th bgcolor=\"$tbl_head_bg_color\"><font size=2>Location</font></th>\n";
print OUTFILE "                    <th bgcolor=\"$tbl_head_bg_color\"><font size=2>Description</font></th>\n";
print OUTFILE "                    <th bgcolor=\"$tbl_head_bg_color\"><font size=2>URL</font></th>\n";
print OUTFILE "                    <th bgcolor=\"$tbl_head_bg_color\"><font size=2>Uptime</font></th>\n";
print OUTFILE "                    <th bgcolor=\"$tbl_head_bg_color\"><font size=2>Contact</font></th>\n";
print OUTFILE "                    <th bgcolor=\"$tbl_head_bg_color\"><font size=2>Software</font></th>\n";
print OUTFILE "                    <th bgcolor=\"$tbl_head_bg_color\"><font size=2>U</font></th>\n";
print OUTFILE "                    <th bgcolor=\"$tbl_head_bg_color\"><font size=2>C</font></th>\n";
print OUTFILE "                    <th bgcolor=\"$tbl_head_bg_color\"><font size=2>G</font></th>\n";
print OUTFILE "                  </tr>\n";

$allservers = 0;
$allusers   = 0;
$allgames   = 0;

foreach $server (@servers) {
    print OUTFILE "                  <tr>\n";
    $/ = "\#\#\n";
    
    # Fix-up for special characters
    $server =~ s/&/&amp;/g;
    $server =~ s/</&lt;/g;
    $server =~ s/>/&gt;/g;
    
    @serverinfo = split /^\#\#\n/m, $server;
    chomp @serverinfo;
    chop @serverinfo;
    
    # put fields into descriptive variable names
    $addr = $serverinfo[0];

    $loc  = $serverinfo[2];
    if (length($loc)<1) {
        $loc = "&nbsp;";
    }
    $desc = $serverinfo[8];
    if (length($desc)<1) {
        $desc = "&nbsp;";
    }
    
    $soft = $serverinfo[3] . "&nbsp;" . $serverinfo[4];
    $osys = $serverinfo[9];
    if (length($osys)<1) {
        $osys = "unknown";
    }
    
    $url = $serverinfo[10];
    
    $contact = $serverinfo[11];
    if (length($contact)<1) {
        $contact = "&nbsp;";
    }
    $email = $serverinfo[12];
    
    $currusers = $serverinfo[5];
    $totusers = $serverinfo[15];
    
    $currchans = $serverinfo[6];
    
    $currgames = $serverinfo[7];
    $totgames = $serverinfo[14];
    
    $uptime = sprintf("%02d:%02d:%02d",int $serverinfo[13]/3600, int ($serverinfo[13]%3600)/60, int $serverinfo[13]%60);
    
    # print it out
    print OUTFILE "                    <td><font size=1><a href=\"bnetd://$addr/\">$addr</a></font></td>\n"; # IP
    print OUTFILE "                    <td><font size=1>$loc</font></td>\n";
    print OUTFILE "                    <td><font size=1>$desc</font></td>\n";
    if (length($url)<1) {
        print OUTFILE "                    <td><font size=1>&nbsp;</font></td>\n";
    } else {
        print OUTFILE "                    <td><a href=\"$url\"><font size=1>$url</font></a></td>\n";
    }
    print OUTFILE "                    <td align=right><font size=1>$uptime</font></td>\n";
    if (length($email)<1) {
        print OUTFILE "                    <td><font size=1>$contact</font></td>\n";
    } else {
        print OUTFILE "                    <td><font size=1>$contact <a href=\"mailto:$email\">($email)</a></font></td>\n";
    }
    print OUTFILE "                    <td><font size=1>$soft<br>$osys</font></td>\n";
    print OUTFILE "                    <td align=right><font size=1>${currusers}c<br>${totusers}t</font></td>\n";
    print OUTFILE "                    <td align=right><font size=1>${currchans}</font></td>\n";
    print OUTFILE "                    <td align=right><font size=1>${currgames}c<br>${totgames}t</font></td>\n";
    print OUTFILE "                  </tr>\n";
    
    $allservers++;
    $allusers+=${currusers};
    $allgames+=${currgames};
}

print OUTFILE "                </table>\n";
print OUTFILE "              </td>\n";
print OUTFILE "            </tr>\n";
print OUTFILE "          </table>\n";
print OUTFILE "          <br><br>\n";
print OUTFILE "        </td>\n";
print OUTFILE "      </tr>\n";

print OUTFILE "      <tr>\n";
print OUTFILE "        <td align=middle><table border=\"0\">\n";
print OUTFILE "          <tr>\n";
print OUTFILE "            <td align=middle bgcolor=\"$sum_bg_color\">\n";
print OUTFILE "              <font size=2 color=\"$sum_text_color\">\n";
print OUTFILE "                servers: " . $allservers . "<br>\n";
print OUTFILE "                users: " . $allusers . "<br>\n";
print OUTFILE "                games: " . $allgames . "<br>\n";
print OUTFILE "                <br>\n";
print OUTFILE "                Last updated " . gmtime(time) . " GMT<br>\n";
print OUTFILE "              </font><br>\n";
print OUTFILE "            </td>\n";
print OUTFILE "          </tr>\n";
print OUTFILE "        </table></td>\n";
print OUTFILE "      </tr>\n";

print OUTFILE "    </table>\n";
print OUTFILE "  </font>\n";
print OUTFILE "</body></html>\n";

close(OUTFILE);

unlink($outfile);
rename($tmpfile, $outfile);

exit(0);

