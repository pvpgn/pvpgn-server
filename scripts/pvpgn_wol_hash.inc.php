<?php
/*
* PHP implementation of the PvPGN WOL Password Hash Algorithm.

* This code is available under the GNU Lesser General Public License:
* http://www.gnu.org/licenses/lgpl.txt

* Original hash funcion Copyright (C)  Luigi Auriemma (aluigi@autistici.org)
*
* Comment:
* Original algorithm starts at offset 0x0041d440 of wchat.dat of 4.221 US version
* The algorithm is one-way, so the encoded password can NOT be completely decoded!

*/
function wol_hash($pass)
{
	if(strlen($pass)<>8)
	{
		return false;
	}
	$pwd1=$pass;
	$esi=8;
	$pwd2=array(null,null,null,null,null,null,null,null);
	$p1=0;
	$p2=0;
	$edx="";
	for($i=0;$i<8;$i++)
	{
		if(ord($pwd1[$p1]) & 1)
		{
			$edx = ord($pwd1[$p1]) << 1;
			$edx &= ord($pwd1[$esi]);
		}
		else
		{
			$edx = ord($pwd1[$p1]) ^ ord($pwd1[$esi]);
		}
		$pwd2[$p2] = $edx;
		$p2++;
		$esi--;
		$p1++;
	}
	
	
	$WOL_HASH_CHAR = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789./";
	$p1 = 0;
	$p2 = 0;
	for($i=0;$i<8;$i++)
	{
		$edx = $pwd2[$p2] & 0x3f;
		$p2++;
		$pwd1[$p1] = $WOL_HASH_CHAR[$edx];
		$p1++;
	}
	return $pwd1;
}

?>