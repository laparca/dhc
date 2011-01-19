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

class CrackersController extends AppController {
	var $name = 'Crackers';
	var $uses = array('Stat');
	var $helpers = array('Html', 'Form');
	
	function index() {
		
	}
	
	function queue() {
		
	}
	
	function overview() {
		//$this->layout = 'cracker';
		
		$now = time();
		$exp = $now - 300;
		
		//dbquery("DELETE FROM `stats` WHERE `updated` < '$exp'");
		$this->Stat->deleteAll(array('Stat.updated <' => '$exp'));
		
	}
	
	function submit() {
		
	}
	
	/**
	* Stores a new has in the database
	*/
	function add() {
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
	
	function output() {
		
	}
	
	function stats() {
		
	}
	
	function beforeFilter() {
		parent::beforeFilter();
		
		$this->set('action', $this->params['action']);
		$this->layout = 'cracker';
	}
}
?>