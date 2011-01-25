<?php
/******************************************************************************
*                                                                             *
* Distributed Hash Cracker v3.0                                               *
*                                                                             *
* Copyright (c) 2009 RPISEC.                                                  *
* Copyright (C) 2010 Samuel Rodríguez Sevilla                                 *
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
******************************************************************************/

class AgentsController extends AppController {
	var $name = 'Agents';
	var $uses = array('WorkUnit', 'Hash', 'Crack');
	var $components = array('BaseN');
	var $helpers = array('Xml');
	
	function getwu() {
		$hostname = mysql_real_escape_string($this->params['url']['hostname']);
		$type = mysql_real_escape_string($this->params['url']['type']);
		$num = intval($this->params['url']['num']);
		
		$now = time();
		
		$alglist = explode(',', $this->params['url']['accept-algorithms']);
		
		// TODO: need to block Workunit, Crack and Hash?
		
		// Look for all expired Workunits
		$result = $this->WorkUnit->find('all', 
			array(
				'conditions' => array('WorkUnit.expiration <' => "$now"),
				'order'      => array('WorkUnit.expiration ASC')
			)
		);

		$orphaned_found = false;

		foreach($result as $workunit) {
			if(in_array($workunit['Crack']['algorithm'], $alglist)) {
				// A expired WU with a compatible algorithm found!
				$this->WorkUnit->updateAll(
					array(
						'WorkUnit.hostname' => "'$hostname'",
						'WorkUnit.devtype' => "'$type'",
						'WorkUnit.devid' => $num,
						'WorkUnit.expiration' => time() + 120 /* two minutes */
					),
					array('WorkUnit.id' => $workunit['WorkUnit']['id'])
				);

				$this->set('info', array(
					'id' => $workunit['WorkUnit']['id'],
					'algorithm' => $workunit['Crack']['algorithm'],
					'charset' => $workunit['Crack']['charset'],
					'start' => $workunit['WorkUnit']['start'],
					'end' => $workunit['WorkUnit']['end'],
					'hashes' => $this->Hash->find('list', array(
						'fields' => array('Hash.id', 'Hash.hash'),
						'conditions' => array('Hash.crack_id' => $workunit['Crack']['id'])
					))
				));

				$orphaned_found = true;
				break;
			}
		}

		if(!$orphaned_found) {
			$crack = $this->Crack->find('first', array(
				'conditions' => array(
					'Crack.active' => 1,
					'Crack.algorithm' => $alglist
				),
				'order' => array(
					'Crack.priority DESC',
					'Crack.started ASC'
				)
			));
//$this->set('test', $crack);
			if(count($crack['Crack']) < 1) {
				$this->render('nowork'); // <nowork reason='idle'></nowork
				return;
			}



			//Get our character set
			$ccode = $crack['Crack']['charset'];
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
			for($i = 0; $i < strlen($ccode); $i++)
				$charset .= $csets[$ccode[$i]];
			$base = strlen($charset);

			//Make reverse charset
			$rcharset = array();
			for($i = 0; $i < $base; $i++)
				$rcharset[$charset[$i]] = $i;	

			//Convert our starting position to a base-N integer
			$nstart = $this->BaseN->make($crack['Crack']['nextwu'], $rcharset);

			//Calculate work unit size
			$wlength = 1000000000;			//Default for GPUs
			if($type == 'core')				
				$wlength = 50000000;		//CPUs get less
			if($crack['Crack']['algorithm'] == 'md5crypt')
				$wlength /= 1000;			//md5crypt gets much smaller WUs

			//Calculate our ending position (inclusive)
			$nend = $this->BaseN->add($nstart, $base, $wlength);

			//Calculate the start of the next work unit
			//$cid = $crack->id;
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
				if(count($nend) == $crack['Crack']['maxlen'])
				{
					$crack['Crack']['active'] = 0;
					$crack['Crack']['updated'] = time();
					foreach($crack['Hash'] as $hash)
						$hash['active'] = 0;
					//dbquery("UPDATE cracks SET `active` = '0', `updated` = '$now' WHERE `id` = '$cid' LIMIT 1");
					//dbquery("UPDATE `hashes` SET `active` = '0' WHERE `crack` = '$cid'");
				}
			}
			else
			{	
				//If not, just add 1.
				$nwu = $this->BaseN->add($nend, $base, 1);
			}
			$start = $crack['Crack']['nextwu'];
			$end = $this->BaseN->toString($nend, $charset);

			//Save start of next WU in table
			$crack['Crack']['nextwu'] = mysql_real_escape_string($this->BaseN->toString($nwu, $charset));
			$crack['Crack']['updated'] = time();

			//Insert the new WU into the table
			$now = time();
			$exp = $now + 120;	//TODO: is 2 minute expiration reasonable?
			$workunit = array('WorkUnit' => array(
				'crack_id' => $crack['Crack']['id'],
				'hostname' => $hostname,
				'devtype' => $type,
				'devid' => $num,
				'start' => $start,
				'end' => $end,
				'started' => time(),
				'expiration' => time() + 120
			));

			$this->Crack->save($crack);
			$this->WorkUnit->save($workunit);
			
			$this->set('info', array(
				'id' => $this->WorkUnit->id,
				'algorithm' => $crack['Crack']['algorithm'],
				'charset' => $crack['Crack']['charset'],
				'start' => $workunit['WorkUnit']['start'],
				'end' => $workunit['WorkUnit']['end'],
				'hashes' => $crack['Hash']
			));
		}
		$this->layout = 'xml';
	}
	
	
	
	
	
	
	function submitwu() {
		//dbquery("LOCK TABLES cracks WRITE, workunits WRITE, stats WRITE, history WRITE, hashes WRITE");

		$id = intval($this->params['url']["wuid"]);
		$collisions = intval($this->params['url']["collisions"]);
		
		
		
		$hashes = array();
		$cleartext = array();
		for($i=0; $i<$collisions; $i++)
		{
			$hashes[$i] = intval($this->params['url']["hash$i"]);
			$cleartext[$i] = mysql_real_escape_string($this->params['url']["cleartext$i"]);
		}

		//Get the crack ID
		//$r = dbquery("SELECT * FROM `workunits` WHERE `id` = '$id' LIMIT 1");
		$workunit = $this->WorkUnit->findById($id);
		//if(mysql_num_rows($r) != 1)		//Report OK since the crack was cancelled
		//	die("ok");
		if(empty($workunit)) {
			$this-render('error');
			return;
		}
		//$wu = mysql_fetch_object($r);
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
}
?>