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
	@file controller.php
	
	@brief Work unit scheduling/dispatching, etc
 */

include 'controller_utils.php';

if($_GET['version'] != "3.2")
{
	die("<nowork reason='version'>\n</nowork>");
}

if($action == 'getwu')
{
	GetWorkUnit();
}
else if($action=='submitwu')
{
	SubmitWorkUnit();
}

/*!
	@brief Generates a work unit and echoes it to the agent.
	
	If any abandoned work units are found, the oldest one is re-issued. If none are present, a new work unit is generated
	and the nextwu variable of the crack is updated appropriately.
	
	If no cracks are in progress, a null work unit is returned.
 */
function GetWorkUnit()
{
	//Get vital statistics
	$hostname = mysql_real_escape_string($_GET['hostname']);
	$type = mysql_real_escape_string($_GET['type']);
	$num = intval($_GET['num']);
	
	//Generate accept-algorithms query
	$alglist = explode(',', $_GET['accept-algorithms']);
	$algq = 'IN(';
	$bFirst = true;
	foreach($alglist as $alg)
	{
		if(!$bFirst)	
			$algq .= ', ';
		$alg = mysql_real_escape_string($alg);
		$algq .= "'$alg'";
		$bFirst = false;
	}
	$algq .= ')';
		
	dbquery("LOCK TABLES cracks WRITE, workunits WRITE, hashes WRITE");
		
	//See if we have any orphaned WUs around
	$now = time();
	$r = dbquery("SELECT * FROM `workunits` WHERE `expiration` < '$now' ORDER BY `expiration` ASC");
	$bFoundWU = false;
	if(mysql_num_rows($r) != 0)
	{
		while(!$bFoundWU)
		{
			//Get the dead WU
			$wu = mysql_fetch_object($r);
			if(!$wu)
				break;
			
			//Look up its crack
			$cid = $wu->crack;
			$r = dbquery("SELECT * FROM `cracks` WHERE `id` = '$cid' AND `algorithm` $algq LIMIT 1");
			if(mysql_num_rows($r) == 0)
				continue;
			$crack = mysql_fetch_object($r);
			
			//Insert the new WU into the table
			$start = $wu->start;
			$end =  $wu->end;
			$exp = time() + 60;	//Expire in 1 minute.
								//(Even a single 2 GHz Opteron core should be able to finish a CPU-sized WU in under 30 seconds.)
								//If work unit size is changed (e.g. to scale to very large grids) expiration should be updated.
			$iq = "INSERT INTO workunits (`crack`, `hostname`, `devtype`, `devid`, `start`, `end`, `expiration`) ";
			$iq .= "VALUES('$cid', '$hostname', '$type', '$num', '$start', '$end', '$exp')";
			dbquery($iq);
			
			//Send it to the agent
			$wuid = mysql_insert_id();
			echo "<workunit>\n";
			echo "   <id>" . $wuid . "</id>\n";
			echo "   <algorithm>" . $crack->algorithm . "</algorithm>\n";
			echo "   <charset>" . $crack->charset . "</charset>\n";
			$s = dbquery("SELECT * FROM `hashes` WHERE `crack` = '$cid'");
			while($h = mysql_fetch_object($s))
				echo '   <hash id=\'' . $h->id . '\'>' . $h->hash . "</hash>\n";		//TODO: skip ones which were already broken
			echo "   <start>" . $wu->start . "</start>\n";
			echo "   <end>" . $wu->end . "</end>\n";
			echo "</workunit>";
			
			//Delete the old WU
			$old = $wu->id;
			dbquery("DELETE FROM `workunits` WHERE `id` = '$old' LIMIT 1");
			
			$bFoundWU = true;
		}
	}
	
	if(!$bFoundWU)
	{
		//Get the crack we are working on (must be active, pick highest priority first, then sort by oldest first)
		$r = dbquery("SELECT * FROM cracks WHERE `active` = '1' AND `algorithm` $algq ORDER BY `priority` DESC, `started` ASC LIMIT 1");
		if(mysql_num_rows($r) == 0)
			die("<nowork reason='idle'>\n</nowork>\n");
		$crack = mysql_fetch_object($r);
		
		$now = time();
		
		//Get our character set
		$ccode = $crack->charset;
		$csets = array(
			'a' => 'abcdefghijklmnopqrstuvwxyz',
			'A' => 'ABCDEFGHIJKLMNOPQRSTUVWXYZ',
			'1' => '1234567890',
			'!' => "!@_-?#$",
			'>' => "`~%^&*()=+[]\\{}|;':\",./<>",
			's' => ' ',
			'n' => "\n"
			);
		$charset = '';
		for($i=0; $i<strlen($ccode); $i++)
			$charset .= $csets[$ccode[$i]];
		$base = strlen($charset);
			
		//Make reverse charset
		$rcharset = array();
		for($i=0; $i<strlen($charset); $i++)
			$rcharset[$charset[$i]] = $i;	

		//Convert our starting position to a base-N integer
		$nstart = MakeBaseN($crack->nextwu, $rcharset);
		
		//Calculate work unit size
		$wlength = 1000000000;			//Default for GPUs
		if($type == 'core')				
			$wlength = 50000000;		//CPUs get less
		if($crack->algorithm == 'md5crypt')
			$wlength /= 1000;			//md5crypt gets much smaller WUs
		
		//Calculate our ending position (inclusive)
		$nend = AddBaseN($nstart, $base, $wlength);
		
		//Calculate the start of the next work unit
		$cid = $crack->id;
		$bSaturated = true;
		$nwu = array();
		for($i=0; $i<count($nend); $i++)
		{
			if($nend[$i] != $base-1)
				$bSaturated = false;
		}
		if($bSaturated)
		{
			//If we saturated, bump length and do {0...0}.
			for($i=0; $i<1 + count($nend); $i++)
				$nwu[$i] = 0;
				
			//If we saturated and are at the end of the search space, mark the crack as exhausted.
			//(If a work unit comes back with success in one of the last WUs, we can change it)
			if(count($nend) == $crack->maxlen)
			{
				dbquery("UPDATE cracks SET `active` = '0', `updated` = '$now' WHERE `id` = '$cid' LIMIT 1");
				dbquery("UPDATE `hashes` SET `active` = '0' WHERE `crack` = '$cid'");
			}
		}
		else
		{	
			//If not, just add 1.
			$nwu = AddBaseN($nend, $base, 1);
		}
		$start = $crack->nextwu;
		$end = BaseNToString($nend, $charset);
		
		//Save start of next WU in table
		$snwu = mysql_real_escape_string(BaseNToString($nwu, $charset));
		dbquery("UPDATE cracks SET `nextwu` = '$snwu', `updated` = '$now' WHERE `id` = '$cid' LIMIT 1");
		
		//Insert the new WU into the table
		$now = time();
		$exp = $now + 120;	//TODO: is 2 minute expiration reasonable?
		$iq = "INSERT INTO workunits (`crack`, `hostname`, `devtype`, `devid`, `start`, `end`, `started`, `expiration`) ";
		$iq .= "VALUES('$cid', '$hostname', '$type', '$num', '$start', '$end', '$now', '$exp')";
		dbquery($iq);
		
		//Print the WU
		$wuid = mysql_insert_id();
		echo "<workunit>\n";
		echo "   <id>" . $wuid . "</id>\n";
		echo "   <algorithm>" . $crack->algorithm . "</algorithm>\n";
		echo "   <charset>" . $crack->charset . "</charset>\n";
		$s = dbquery("SELECT * FROM `hashes` WHERE `crack` = '$cid'");
		while($h = mysql_fetch_object($s))
			echo '   <hash id=\'' . $h->id . '\'>' . $h->hash . "</hash>\n";		//TODO: skip ones which were already broken
		echo "   <start>" . $start . "</start>\n";
		echo "   <end>" . $end . "</end>\n";
		echo "</workunit>";
	}
	
	dbquery("UNLOCK TABLES");
}

/*!
	@brief Processes a completed work unit.
 */
function SubmitWorkUnit()
{
	dbquery("LOCK TABLES cracks WRITE, workunits WRITE, stats WRITE, history WRITE, hashes WRITE");
	
	$id = intval($_GET["wuid"]);
	$collisions = intval($_GET["collisions"]);
	$hashes = array();
	$cleartext = array();
	for($i=0; $i<$collisions; $i++)
	{
		$hashes[$i] = intval($_GET["hash$i"]);
		$cleartext[$i] = mysql_real_escape_string($_GET["cleartext$i"]);
	}
	
	//Get the crack ID
	$r = dbquery("SELECT * FROM `workunits` WHERE `id` = '$id' LIMIT 1");
	if(mysql_num_rows($r) != 1)		//Report OK since the crack was cancelled
		die("ok");
	$wu = mysql_fetch_object($r);
	$cid = $wu->crack;
	
	//Clean out stats not updated in 2 mins
	$now = time();
	$exp = $now - 120;
	dbquery("DELETE FROM `stats` WHERE `updated` < '$exp'");
	$exp2 = $now - 900;
	dbquery("DELETE FROM `history` WHERE `time` < '$exp2'");
	
	//Get stats
	$dt = floatval($_GET["dt"]);
	$speed = floatval($_GET["speed"]);
	$host = mysql_real_escape_string($wu->hostname);
	$type = mysql_real_escape_string($wu->devtype);
	$dev = $wu->devid;
	
	//See if we already have history for this timestamp
	//TODO: Make this more efficient
	$r = dbquery("SELECT * FROM `history` WHERE `time` = '$now' LIMIT 1");
	if(mysql_num_rows($r) == 0)
	{	
		$fspeed = 0;
		$r = dbquery("SELECT * FROM stats");
		while($line = mysql_fetch_object($r))
		{
			$fspeed += $line->speed;
		}
		
		//Add to global stats
		dbquery("INSERT INTO `history` VALUES('$now', '$fspeed')");
	}
	
	//Delete all old stats for our current device
	dbquery("DELETE FROM `stats` WHERE `device` = '$host-$type-$dev'");
	
	//Add us to stats
	dbquery("INSERT INTO `stats` VALUES('$now', '$host-$type-$dev', '$dt', '$speed')");
	
	//Update the crack
	if($collisions > 0)
	{
		for($i=0; $i<$collisions; $i++)
		{
			$hid = $hashes[$i];
			$collision = $cleartext[$i];
			dbquery("UPDATE `hashes` SET `collision` = '$collision', `active` = '0' WHERE `id` = '$hid' LIMIT 1");
		}
		dbquery("UPDATE `cracks` SET `updated` = '$now' WHERE `id` = '$cid'");
	}
	
	$r = dbquery("SELECT * FROM `hashes` WHERE `active` = '1' AND `crack` = '$cid'");
	if(mysql_num_rows($r) == 0)
	{
		//No hashes left? Close the crack
		dbquery("UPDATE `cracks` SET `active` = '0' WHERE `id` = '$cid'");
	}
	
	//Delete the work unit AFTER updating the crack in case of a server segfault or something
	//Repeating a WU is better than missing one
	dbquery("DELETE FROM `workunits` WHERE `id` = '$id' LIMIT 1");
	
	//Show status
	echo "ok";
	
	dbquery("UNLOCK TABLES");	
}
?>
