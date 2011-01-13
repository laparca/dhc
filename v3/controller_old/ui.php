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
	@file ui.php
	
	@brief User interface for the controller
*/

//Validate page ID
$page = $_GET['page'];
if($page=='')
	$page='overview';
	
require_once('controller_utils.php');
	
//Set up page title
$titles = array(
	'overview' => 'Grid Overview',
	'queue' => 'Crack Queue',
	'submit' => 'Submit Hash',
	'output' => 'Results',
	'stats' => 'Crack Stats',
	'settings' => 'Server Settings'
	);
$title = $titles[$page];
$host = $_SERVER['SERVER_NAME'];

//Generate menu
$links = array(
	'overview' => 'Overview',
	'queue' => 'View Queue',
	'submit' => 'Submit',
	'output' => 'Output',
	'stats' => 'Stats',
	'settings' => 'Settings'
	);
	
$menu = '';

foreach($links as $pg => $txt)
{
	$c = 'taboff';
	if($pg == $page)
		$c = 'tabon';
	//$menu .= "<a class='$c' href='?page=$pg'>$txt</a>\n";
	$menu .= "<li class='$c'><a href='?page=$pg'>$txt</a></li>";
}

//Set up templating
$tarr = array(
	'title' => $title,
	'host' => $host,
	'menu' => $menu
);


//Process the page-specific stuff
if($page == 'queue')
{
	$tarr['ovrows'] = DoQueue();	
	$tarr['body'] = GetTemplatedPage('data/queue.html', $tarr);
}

else if($page == 'overview')
{
	//Clean out old stats
	$now = time();
	$exp = $now - 300;
	dbquery("DELETE FROM `stats` WHERE `updated` < '$exp'");
	
	$tarr['body'] = GetTemplatedPage('data/overview.html', $tarr);
}

else if($page == 'submit')
{
	$tarr['body'] = GetTemplatedPage('data/submit.html', $tarr);
}

else if($page == 'output')
{
	$tarr['oprows'] = DoOutput();
	$tarr['body'] = GetTemplatedPage('data/output.html', $tarr);
}

else if($page == 'stats')
{
	$now = time();
	
	//Generate stuff
	$r = dbquery('SELECT DISTINCT `device`, `updated`, `speed` FROM `stats');
	$stats = array();
	while($line = mysql_fetch_object($r))
	{
		$xr = dbquery("SELECT * FROM `stats` WHERE `device` = '$line->device' ORDER BY `updated` DESC");
		$xline = mysql_fetch_object($xr);
		
		//Parse things out
		$arr = explode('-', $xline->device);
		$hostname = $arr[0];
		$type = $arr[1];
		$num = $arr[2];
		$speed = $xline->speed;
		$dt = $now - $xline->updated;
		
		$dev = "$type $num";
		
		//Save stuff
		$stats[$hostname][$dev]['speed'] = $speed;
		$stats[$hostname][$dev]['dt'] = $dt;
	}

	$srows = '';
	foreach($stats as $hostname => $stuff)
	{
		$srows .= '<tr style=\'vertical-align:top\'>';
		$srows .= '<td>' . $hostname . '</td>';
		
		$devs = '';
		$speeds = '';
		$dts = '';
		
		$tspeed  = '';
		foreach($stuff as $device => $things)
		{
			$devs .= $device . '<br/>';
			$speeds .= $things['speed'] . '<br/>';
			$tspeed += $things['speed'];
			$dts .= $things['dt'] . ' seconds ago<br/>';
		}
		
		$devs .= 'Total<br/>';
		$speeds .= $tspeed . '<br/>';
		
		$srows .= '<td>' . $devs . '</td><td>' . $speeds . '</td><td>' . $dts . '</td>';		
		$srows .= "</tr>\n";
	}
	
	$tarr['srows'] = $srows;
	$tarr['body'] = GetTemplatedPage('data/stats.html', $tarr);
}

//Set up output
RenderPage($tarr);


/*!
	@brief Renders the "queue" page of the controller and processes submissions
	
	@return Page content
 */
function DoQueue()
{
	$rows = '';
	
	//Submit a new hash if requested
	$op = $_GET['op'];
	if($op == 'submit')
	{
		$alg = $_POST['alg'];
		$hashes = explode("\n", $_POST['hash']);
		$len = intval($_POST['len']);
		$exp = intval($_POST['exp']);
		$pri = intval($_POST['pri']);
		
		//Should never have any angle brackets but make sure
		for($i=0; $i<count($hashes); $i++)
			$hashes[$i] = strip_tags($hashes[$i]);
		
		//Validate input
		$bOK = true;
		if($pri < 0 || $pri > 10)
		{
			$bOK = false;
			echo "Invalid priority level<br/>";
		}
		if($exp != 0)
			$exp = time() + 60*$exp;
		if($exp == 0)
			$exp = time() + 10*365*24*60*60;
		if($len < 0 || $len > 32)
		{
			$bOK = false;
			echo "Invalid length<br/>";
		}
		$hash_lengths = array(
			'md5' => 32,
			'md4' => 32,
			'ntlm' => 32,
			'sha1' => 40,
			'sha256' => 64,
			'md5crypt' => -1	/* TODO: do something here */
			);
		$hashlen = $hash_lengths[$alg];
		if($alg != 'md5crypt')
		{
			//Check length against array
			//TODO: handle md5crypt variable salt lengths later
			foreach($hashes as $hash)
			{			
				$hash = trim($hash);
				if(strlen($hash) != $hashlen)
				{
					$bOK = false;
					$len = strlen($hash);
					echo "Invalid hash length $len on hash $hash (expected $hashlen)<br/>";
				}
			}
		}
		if(!isset($hash_lengths[$alg]))
		{
			echo "Invalid hash algorithm";
			$bOK = false;
		}
		
		//No batch cracking of md5crypt allowed since it's salted (we gain nothing)
		if($alg == 'md5crypt' && count($hashes) != 1)
		{
			echo "Batch cracking of md5crypt is not possible.";
			$bOK = false;
		}
		
		//GPUs have limited memory!
		if(count($hashes) > 128)
		{
			echo "The current implementation is limited to 128 simultaneous hashes.";
			$bOK = false;
		}
		
		if($bOK)
		{
			$alg = mysql_real_escape_string($alg);
						
			//Set charset
			//lower upper nums symbols space newline other
			$cset = '';
			if($_POST['lower'] == 'on')
				$cset .= 'a';
			if($_POST['upper'] == 'on')
				$cset .= 'A';
			if($_POST['nums'] == 'on')
				$cset .= '1';
			if($_POST['somesymbols'] == 'on')
				$cset .= '!';
			if($_POST['allsymbols'] == 'on')
				$cset .= '>';
			if($_POST['space'] == 'on')
				$cset .= 's';
			if($_POST['newline'] == 'on')
				$cset .= 'n';
				
			//Set next WU
			$nwu = $cset[0];
			if($nwu == 's')
				$nwu = ' ';
			if($nwu == 'n')
				$nwu = '\n';
			
			//Insert the crack
			$tm = time();
			$q = 'INSERT INTO `cracks` (`algorithm`,`charset`,`maxlen`,`nextwu`,`started`,`expiration`,`priority`, `active`, `updated`) VALUES(';
			$q .= "'$alg', '$cset', '$len', '$nwu', '$tm', '$exp', '$pri', '1', '$tm'";
			$q .= ')';
			dbquery($q);
			$crack = mysql_insert_id();
			
			//Add hashes
			foreach($hashes as $hash)
			{
				//Sanitize input
				$hash = mysql_real_escape_string(trim($hash));
				
				//Create a new hash associated with this crack
				$q = 'INSERT INTO `hashes` (`crack`, `hash`, `collision`, `active`) VALUES(';
				$q .= "'$crack', '$hash', '', '1'";
				$q .= ')';
				dbquery($q);
			}
		}
	}
	
	if($op == 'cancel')
	{
		$id = intval($_GET['id']);
		dbquery("DELETE FROM `cracks` WHERE `id` = '$id' LIMIT 1");
		dbquery("DELETE FROM `workunits` WHERE `crack` = '$id' LIMIT 1");
	}
	
	//Process rows
	$r = dbquery("SELECT * FROM `cracks` WHERE `active` = '1'");
	while($row = mysql_fetch_object($r))
	{
		$time = time();
		$rows .= '<tr id=\'' . $row->hash . '\'>';
		$rows .= '<td><a href=\'?page=queue&op=cancel&id=' . $row->id . '\'>[Cancel]</a></td>';
		$rows .= '<td>' . $row->algorithm . '</td>';
		$rows .= '<td>' . $row->charset . '</td>';
		
		$hashes = '';
		$s = dbquery('SELECT `hash` FROM `hashes` WHERE `crack` = \'' . $row->id . '\' AND `collision` = \'\'');
		while($hash = mysql_fetch_object($s))
			$hashes .= $hash->hash . "\n";		
		
		$rows .= '<td>' . $hashes . '</td>';
		$rows .= '<td>' . $row->maxlen . '</td>';
		$rows .= '<td>' . $row->nextwu . '</td>';
		$rows .= '<td>' . date('Hi Ymd', $row->started) . '</td>';
		$rows .= '<td>' . date('Hi Ymd', $row->expiration) . '</td>';
		$rows .= '<td>' . $row->priority . '</td>';
		$frac=GetStatsPercentage($row->id);
		$rows .= "<td><img src='index.php?action=ajax&page=progress&percentage=$frac' title='$frac% complete'/></td>";
		$rows .= "</tr>\n";
	}
	
	return $rows;
}

/*!
	@brief Renders the "output" page of the controller
	
	@return Page content
 */
function DoOutput()
{
	$rows = '';
	
	$r = dbquery("SELECT * FROM `hashes` WHERE `active` = '0' ORDER BY `id` DESC");
	while($row = mysql_fetch_object($r))
	{
		$cid = $row->crack;
		$s = dbquery("SELECT * FROM `cracks` WHERE `id` = '$cid'");
		$crack = mysql_fetch_object($s);
		
		$rows .= '<tr>';
		$rows .= '<td>' . '' . '</td>';	//TODO: add command links
		$rows .= '<td>' . $crack->algorithm . '</td>';
		$rows .= '<td>' . $crack->charset . '</td>';
		$rows .= '<td>' . $row->hash . '</td>';
		$status = 'Search space exhausted';
		if($row->collision != '')
			$status = 'Cracked';
		else if($crack->updated > $crack->expiration)
			$statusd = 'Time expired';
		$rows .= '<td>' . $status . '</td>';
		$rows .= '<td>' . $crack->maxlen . '</td>';
		$rows .= '<td>' . $row->collision .'</td>';
		$rows .= "</tr>\n";
	}
	
	return $rows;
}

?>
