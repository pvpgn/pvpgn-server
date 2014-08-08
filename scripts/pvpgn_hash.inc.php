<?php
/*
** PHP implementation of the PvPGN Password Hash Algorithm.

** Copyright 2004 Aaron (aaron@pvpgn.org)

** This code is available under the GNU Lesser General Public License:
** http://www.gnu.org/licenses/lgpl.txt

** This code in general is based on:
** Copyright 2002 - 2003 Marcus Campbell
** http://www.tecknik.net/sha-1/
** Based on the JavaScript SHA-1 implementation by Paul Johnston
** http://pajhome.org.uk/

** the safe_rol function is taken from an PHP SHA-1 implementation
** written by Chris Monson (chris@bouncingchairs.net)
** Most recent version available on http://bouncingchairs.net
** (Based on the SHA algorithm as given in "Applied Cryptography")

*/
function str2blks_pvpgn($str) {
	$nblk = ((strlen($str) + 8) >> 6) + 1;
	
	for($i=0; $i < $nblk * 16; $i++) $blks[$i] = 0;
	for($i=0; $i < strlen($str); $i++) {
		$blks[$i >> 2] |= ord(substr($str, $i, 1)) << ( ($i % 4) * 8);
	}
	return $blks;
}
function safe_add($x, $y) {
	$lsw = ($x & 0xFFFF) + ($y & 0xFFFF);
	$msw = ($x >> 16) + ($y >> 16) + ($lsw >> 16);
	return ($msw << 16) | ($lsw & 0xFFFF);
}

function safe_not( $num ) {
	$lsw = (~($num & 0xFFFF)) & 0xFFFF;
	$msw = (~($num >> 16)) & 0xFFFF;
	return ($msw << 16)  | $lsw;
}

function safe_rol( $num, $amt ) {
        $leftmask = 0xffff | (0xffff << 16);
        $leftmask <<= 32 - $amt;
        $rightmask = 0xffff | (0xffff << 16);
        $rightmask <<= $amt;
        $rightmask = safe_not($rightmask);

        $remains = $num & $leftmask;
        $remains >>= 32 - $amt;
        $remains &= $rightmask;

        $res = ($num << $amt) | $remains;

        return $res;
}

function ft($t, $b, $c, $d) {
	if($t < 20) return ($b & $c) | ((safe_not($b)) & $d);
	if($t < 40) return $d ^ $c ^ $b;
	if($t < 60) return ($c & $b) | ($d & $c) | ($d & $b);
	return $d ^ $c ^ $b;
}

function kt($t) {
	if ($t < 20) {
		return  0x5a82 << 16 | 0x7999;
	} else if ($t < 40) {
		return  0x6ed9 << 16 | 0xeba1;
	} else if ($t < 60) {
		return  0x8f1b << 16 | 0xbcdc;
	} else {
		return  0xca62 << 16 | 0xc1d6;
	}
}
function pvpgn_hash($str) {
	$x = str2blks_pvpgn($str);
		
	$a =  0x6745 << 16 | 0x2301;
	$b =  0xefcd << 16 | 0xab89;
	$c =  0x98ba << 16 | 0xdcfe;
	$d =  0x1032 << 16 | 0x5476;
	$e =  0xc3d2 << 16 | 0xe1f0;	
	
	for($i = 0; $i < sizeof($x); $i += 16) {
		$olda = $a;
		$oldb = $b;
		$oldc = $c;
		$oldd = $d;
		$olde = $e;
		
		for($j = 0; $j < 16; $j++) {		
			$w[$j] = $x[$i+$j];
		}

		for($j = 0; $j < 64; $j++) {		
			$ww = $w[$j] ^ $w[$j+8] ^ $w[$j+2] ^ $w[$j+13];
			$w[$j+16] = 1 << ($ww%32);
		}
		
		for($j = 0; $j < 80; $j++) {
			if ($j<20)
			{
				$t = safe_add(safe_add(safe_rol($a, 5), ft($j, $b, $c, $d)), safe_add(safe_add($e, $w[$j]), kt($j)));
			}
			else
			{
				$t = safe_add(safe_add(safe_rol($t, 5), ft($j, $b, $c, $d)), safe_add(safe_add($e, $w[$j]), kt($j)));
			}
			$e = $d;
			$d = $c;
			$c = safe_rol($b, 30);
			$b = $a;
 			$a = $t;
		}
		$a = safe_add($t, $olda);
		$b = safe_add($b, $oldb);
		$c = safe_add($c, $oldc);
		$d = safe_add($d, $oldd);
		$e = safe_add($e, $olde);
	}
	return sprintf("%08x%08x%08x%08x%08x",$a,$b,$c,$d,$e);
}

?>