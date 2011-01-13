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
	@file ajaxserver.php
	
	@brief AJAX server
 */
 
require_once('controller_utils.php');

$page = $_GET['page'];

if($page == 'overview')
{
	ShowOverview();
}
else if($page == 'stats')
{
	ShowStatsImage();
}
else if($page == 'progress')
{
	ShowProgressImage();
}

/*!
	@brief Gets dynamic content for the "overview" page
 */
function ShowOverview()
{
	//Get changes made since this time
	$from = intval($_GET["from"]) - 1;
	
	//Print the current time
	echo time() . "\n";
	
	//Get changes
	$r = dbquery("SELECT * FROM `cracks` WHERE `updated` >= '$from'");
	while($row = mysql_fetch_object($r))
	{
		if($row->disposition == 'a' && $row->updated == $row->started)
		{
			//Active crack, newly added - create new row
			echo 'add,';
			echo $row->id . ',';
			echo $row->algorithm . ',';
			echo $row->charset . ',';
			echo $row->hash . ',';
			echo $row->maxlen . ',';
			echo $row->nextwu . ',';
			echo date('Hi Ymd', $row->started) . ',';
			echo date('Hi Ymd', $row->expiration) . ',';
			echo '0';
			echo $row->priority;
		}
		else if($row->disposition == 'a')
		{
			//Active crack, updated - update the row
			$time = time();
			echo 'update,';
			echo $row->id . ',';
			echo $row->algorithm . ',';
			echo $row->charset . ',';
			echo $row->hash . ',';
			echo $row->maxlen . ',';
			echo $row->nextwu . ',';
			echo date('Hi Ymd', $row->started) . ',';
			echo date('Hi Ymd', $row->expiration) . ',';
			echo $row->priority . ',';
			$frac=GetStatsPercentage($row->id);
			echo "<img src='index.php?action=ajax&page=progress&percentage=$frac' title='$frac% complete'/>";
		}
		else if($row->disposition == 'c')
		{
			//Crack completed - remove the row
			echo 'delete,';
			echo $row->hash;
		}
		echo "\n";
	}
}

/*!
	@brief Gets the latest cell of the stats graph
 */
function ShowStatsImage()
{
	$barheight = ($speed / 15) + 1;
	$barheight = min($barheight, 300);
	
	//Clean out old history
	$now = time();
	$exp = $now - 800;
	dbquery("DELETE FROM `history` WHERE `time` < '$exp'");
	
	//Get the info
	$r = dbquery("SELECT * FROM `history` ORDER BY `time` ASC");
	$lines = array();
	$maxspeed = 0;
	while($line = mysql_fetch_object($r))
	{
		array_push($lines, $line);
		$maxspeed = max($maxspeed, $line->speed);
	}
		
	//Format the image
	$height = 300;
	$width = 900;
	$bodyheight = $height - 20;	
	$img = imagecreatetruecolor($width, $height);
	$bg = imagecolorallocate($img, 255, 255, 255);
	$black = imagecolorallocate($img, 0x50, 0x50, 0x50);
	$fg = imagecolorallocate($img, 0x00, 0x33, 0x66);
	$lmargin = 50;
	imagefill($img, 0, 0, $bg);
	
	$units = "MHz";
	$multiplier = 1;
	if($maxspeed < 1)
	{
		$units = "KHz";
		$multiplier = 1000;
	}
	if($maxspeed > 1000)
	{
		$units = "GHz";
		$multiplier = 0.001;
	}
	
	//Draw axes and tickmarks
	imageline($img, $lmargin, $height-10, $width, $height-10, $black);
	imageline($img, $lmargin, 10, $lmargin, $height-10, $black);
	for($x=($width-60); $x >= $lmargin; $x -= 60)
	{
		imageline($img, $x, $height-15, $x, $height-10, $black);
		$m = ($width-$x) / 60;
		imagestring($img, 1, $x, $height - 8, "$m min", $black);
	}
	for($p=0; $p<1; $p+=0.1)
	{
		$y = $height-($p*$bodyheight);
		imageline($img, $lmargin, $y, $lmargin+5, $y, $black);
		$str = sprintf("%.2f $units", $p*$maxspeed*$multiplier);
		imagestring($img, 1, 0, $y, $str, $black);
	}
	
	//Draw the line
	$prevx = -1;
	$prevy = -1;
	foreach($lines as $line)
	{
		//X axis: 1 pixel per second, measured from right side.
		//We draw left to right.
		$dt = $now - $line->time;
		$x = ($width - $dt) + 20;
		
		if($x > $width)
			continue;
		if($x < $lmargin)
			continue;
		
		//Y axis: auto scaling
		$speedfrac = $line->speed / $maxspeed;
		$y = 10 + ( $bodyheight * (1 - $speedfrac) );
		
		//If we have no previous state, don't draw
		if($prevx >= 0)
			imageline($img, $x, $y, $prevx, $prevy, $fg);
		
		//Save state
		$prevx = $x;
		$prevy = $y;
	}
	
	//Print it
	header("Content-Type: image/png");
	imagepng($img);
}

/*!
	@brief Gets a progress bar
 */
function ShowProgressImage()
{
	//Look up the crack
	$frac = intval($_GET['percentage']);
		
	//Format the image
	header("Content-Type: image/png");
	$height = 16;
	$width = 100;
	$img = imagecreatetruecolor($width, $height);
	$bg = imagecolorallocate($img, 255, 255, 255);
	$fg = imagecolorallocate($img, 0x00, 0x33, 0x66);
	imagefill($img, 0, 0, $bg);
	imagefilledrectangle($img, 0, $height, $width*$frac/100, 0, $fg);
	
	//Print it
	imagepng($img);
}
?>
