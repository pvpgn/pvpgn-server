#!/usr/bin/awk -f

# This file converts tcpdump output into a hexdump with
# ASCII strings on the right.  It replaces packet headers
# with "*"s to reduce visual noise.
#
# Use it like:
#  tcpdump -s 2000 -x [<filter options>] | reform.awk
# To save a dump:
#  tcpdump -s 2000 -w dumpfile
# To replay it: 
# tcpdump -x -s 2000 -r dumpfile | reform.awk


function hexvalue(nybble) {
    if (nybble=="0") { return  0; }
    if (nybble=="1") { return  1; }
    if (nybble=="2") { return  2; }
    if (nybble=="3") { return  3; }
    if (nybble=="4") { return  4; }
    if (nybble=="5") { return  5; }
    if (nybble=="6") { return  6; }
    if (nybble=="7") { return  7; }
    if (nybble=="8") { return  8; }
    if (nybble=="9") { return  9; }
    if (nybble=="a") { return 10; }
    if (nybble=="b") { return 11; }
    if (nybble=="c") { return 12; }
    if (nybble=="d") { return 13; }
    if (nybble=="e") { return 14; }
    if (nybble=="f") { return 15; }
}


#                         ab67 483d cac9 08ca 3efb 35f8 9406 36e8
{
    if (substr($0,1,1)=="\t") {
        printf("    ");
        str = "";
        for (i=1; i<=NF; i++) {
            nybble1 = substr($(i),1,1);
            nybble2 = substr($(i),2,1);
            byte1 = 16*hexvalue(nybble1)+hexvalue(nybble2);
            if (headerleft) {
                headerleft--;
                printf("** ");
                str = str " ";
            } else {
                printf("%02X ",byte1);
                if (byte1<32 || byte1>126) {
                    str = str ".";
                } else {
                    str = str sprintf("%c",0+byte1);
                }
            }
            
            nybble3 = substr($(i),3,1);
            nybble4 = substr($(i),4,1);
            if (nybble3=="" && nybble4=="") {
                printf("   ");
            } else {
                byte2 = 16*hexvalue(nybble3)+hexvalue(nybble4);
                if (headerleft) {
                    headerleft--;
                    printf("** ");
                    str = str " ";
                } else {
                    printf("%02X ",byte2);
                    if (byte2<32 || byte2>126) {
                        str = str ".";
                    } else {
                        str = str sprintf("%c",0+byte2);
                    }
                }
            }
            if (i==4) {
                printf("  ");
            }
        }
        for (; i<=8; i++) {
            if (i==4) {
                printf("  ");
            }
            printf("      ");
        }
        printf("   %s\n",str);
    } else {
        printf("%s\n",$0);
        headerleft = 40;
    }
}

