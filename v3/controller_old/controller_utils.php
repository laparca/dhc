<?php
/******************************************************************************
*                                                                             *
* Distributed Hash Cracker v3.0                                               *
*                                                                             *
* Copyright (c) 2009 RPISEC.                                                  *
* All rights reserved.                                                        *
*                                                                             *
* Redistribution and use in source and binary forms, with or without modifi-  *
* cation, are permitted provided that the following conditions are met:       *
*                                                                             *
*    * Redistributions of source code must retain the above copyright notice  *
*      this list of conditions and the following disclaimer.                  *
*                                                                             *
*    * Redistributions in binary form must reproduce the above copyright      *
*      notice, this list of conditions and the following disclaimer in the    *
*      documentation and/or other materials provided with the distribution.   *
*                                                                             *
*    * Neither the name of RPISEC nor the names of its contributors may be    *
*      used to endorse or promote products derived from this software without *
*      specific prior written permission.                                     *
*                                                                             *
* THIS SOFTWARE IS PROVIDED BY RPISEC "AS IS" AND ANY EXPRESS OR IMPLIED      *
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF        *
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN     *
* NO EVENT SHALL RPISEC BE HELD LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  *
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED    *
* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR      *
* PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF      *
* LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING        *
* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS          *
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                *
*                                                                             *
*******************************************************************************/

/*!
	@file controller_utils.php
	
	@brief Helper functions used by controller back end
 */
 
/*!
	@brief Parses a base-N object out of a string and a reverse character set
	
	@param $str Input string
	@param $rcharset Reverse character set
	
	@return Base-N index array
 */
function MakeBaseN($str, $rcharset)
{
	$ret = array();
	for($i=0; $i<strlen($str); $i++)
		$ret[$i] = $rcharset[$str[$i]];
	return $ret;
}

/*!
	@brief Adds an integer to a base-N object (using ripple carry)
	
	@param $num Input array
	@param $base Base of $num
	@param $add Value to add to $num
	
	@return Base-N index array
 */
function AddBaseN($num, $base, $add)
{
	$len = count($num);
	
	//Get number of digits required to add this number
	$maxplace = ceil( log($add) / log($base) );
	
	//Too much? Can't possibly fit, saturate and quit
	if($maxplace > $len)
	{
		for($i=0; $i<$len; $i++)
			$num[$i] = $base-1;
		return $num;
	}
	
	//No, we will fit. Bump the rightmost digit
	$num[$len-1] += $add;
	
	//Handle carries
	$bOverflow = false;
	$pvalue = 1;
	for($i=$len-1; $i>=0; $i--)
	{
		//Less than base? We're done
		if($num[$i] < $base)
			break;
			
		//Greater than base? Handle carry
		$low = $num[$i] % $base;
		$high = floor($num[$i] / $base);
		$num[$i] = $low;
		
		//Carry off end? Overflow
		if($high != 0 && $i==0)
		{
			$bOverflow = true;
			break;
		}
		
		$num[$i-1] += $high;
	}
	
	//Saturate in case of overflow
	if($bOverflow)
	{
		for($i=0; $i<$len; $i++)
			$num[$i] = $base-1;
	}
	
	//Done
	return $num;
}

/*!
	@brief Converts a base-N integer to a string
	
	@param $num Input array
	@param $charset Character set
	
	@return Output string
 */
function BaseNToString($num, $charset)
{
	$str = '';
	for($i=0; $i<count($num); $i++)
		$str .= $charset[$num[$i]];
	return $str;
}

/*!
	@brief Converts a base-N integer to a float
		
	@param $num Input array
	@param $charset Character set
	
	@return Output value
 */
function BaseNToFloat($num, $charset)
{
	$ret = 0;
	$len = count($num);
	$base = strlen($charset);
	
	for($i=0; $i<$len; $i++)
	{
		$pval = pow($base, $len-1-$i);
		$ret += ($num[$i]+1) * $pval;
	}
		
	return $ret;
}

function GetStatsPercentage($crack)
{
	$r = dbquery("SELECT * FROM `cracks` WHERE `id` = '$crack'");
	
	//Get progress
	$line = mysql_fetch_object($r);
	
	//Get charset
	$ccode = $line->charset;
	$csets = array(
		'a' => 'abcdefghijklmnopqrstuvwxyz',
		'A' => 'ABCDEFGHIJKLMNOPQRSTUVWXYZ',
		'1' => '1234567890',
		'!' => "`~!@#$%^&*()-=_+[]\\{}|;':\",./<>?",
		's' => ' ',
		'n' => "\n"
		);
	$charset = '';
	for($i=0; $i<strlen($ccode); $i++)
		$charset .= $csets[$ccode[$i]];
		
	//Parse data
	$rcharset = array();
	for($i=0; $i<strlen($charset); $i++)
		$rcharset[$charset[$i]] = $i;	
	$bcur = MakeBaseN($line->nextwu, $rcharset);
	$bend = array();
	for($i=0; $i<$line->maxlen; $i++)
		$bend[$i] = strlen($charset)-1;
	
	//Calculate progress
	$cur = BaseNToFloat($bcur, $charset);
	$end = BaseNToFloat($bend, $charset);
	if($end != 0)
		$frac = $cur / $end;
		
	return sprintf('%.2f',$frac*100);
}
?>
