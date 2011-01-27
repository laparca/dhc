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
	var $uses = array('Stat', 'Crack', 'Hash', 'WorkUnit');
	var $helpers = array('Html', 'Form', 'Session');
	
	function index() {
		
	}
	
	function queue() {
		$this->set('cracks', $this->Crack->findAllByActive(1));
	}
	
	function overview() {
		//$this->layout = 'cracker';
		
		$now = time();
		$exp = $now - 300;
		
		//dbquery("DELETE FROM `stats` WHERE `updated` < '$exp'");
		$this->Stat->deleteAll(array('Stat.updated <' => '$exp'));
		
	}
	
	function submit() {
		if(!empty($this->data)) {
			$this->set('test', $this->data);

			$alg = $this->data['Cracker']['alg'];
			$hashes = explode("\n", $this->data['Cracker']['hashes']);
			$len = intval($this->data['Cracker']['len']);
			$exp = intval($this->data['Cracker']['exp']);
			$pri = intval($this->data['Cracker']['pri']);

			//Should never have any angle brackets but make sure
			for($i=0; $i<count($hashes); $i++)
				$hashes[$i] = strip_tags($hashes[$i]);
					//Validate input
			$bOK = true;
			if($pri < 0 || $pri > 10)
			{
				$bOK = false;
				$this->set('priError', __("Invalid priority level", true));
			}
			if($exp != 0)
				$exp = time() + 60*$exp;
			if($exp == 0)
				$exp = time() + 10*365*24*60*60;
			if($len < 0 || $len > 32)
			{
				$bOK = false;
				$this->set('lenError', __("Invalid length", true));
			}
			$hash_lengths = array(
				'md5' => 32,
				'md4' => 32,
				'ntlm' => 32,
				'sha1' => 40,
				'sha256' => 64,
				'md5crypt' => -1	// TODO: do something here
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
						$error_msg = __("Invalid hash length %d on hash %s (expected %d)", true);
						$this->set('hashesError', sprintf($error_msg, $len, $hash, $hashlen));
					}
				}
			}
			if(!isset($hash_lengths[$alg]))
			{
				$this->set('algError', __('Invalid hash algorithm', true));
				$bOK = false;
			}

			//No batch cracking of md5crypt allowed since it's salted (we gain nothing)
			if($alg == 'md5crypt' && count($hashes) != 1)
			{
				$this->set('algError', __("Batch cracking of md5crypt is not possible.", true));
				$bOK = false;
			}

			//GPUs have limited memory!
			if(count($hashes) > 128)
			{
				$this->set('hashesError', __("The current implementation is limited to 128 simultaneous hashes."));
				$bOK = false;
			}

			if($bOK)
			{
				$alg = mysql_real_escape_string($alg);
				$character_set = array(
					'lower' => 'a',
					'upper' => 'A',
					'nums' => '1',
					'somesymbols' => '!',
					'allsymbols' => '>',
					'space' => 's',
					'newline' => 'n'
				);

				$cset = '';
				foreach($this->data['Cracker']['chset'] as $ch) {
					if(isset($character_set[$ch]))
						$cset .= $character_set[$ch];
				}

				//Set next WU
				$nwu = $cset[0];
				if($nwu == 's')
					$nwu = ' ';
				if($nwu == 'n')
					$nwu = '\n';

				$ins_hashes = array();
				foreach($hashes as $hash)
				{
					//Sanitize input
					$hash = mysql_real_escape_string(trim($hash));

					$ins_hashes[] = array(
						'hash' => $hash,
						'collision' => '',
						'active' => '1');
				}


				$tm = time();
				$this->Crack->saveAll(array(
					'Crack' => array(
						'algorithm' => $alg,
						'charset' => $cset,
						'maxlen' => $len,
						'nextwu' => $nwu,
						'started' => $tm,
						'expiration' => $exp,
						'priority' => $pri,
						'active' => '1',
						'updated' => $tm
					),
					'Hash' => $ins_hashes
				));
				
				$this->Session->setFlash(__('Information saved', true));
			}

		}
	}
		
	function output() {
		$this->set('hashes', $this->Hash->findAllByActive(0, array(), array('Hash.id' => 'DESC')));
	}
	
	function stats() {
		$this->set('stats', $this->Stat->find('all', array(
			'order' => array(
				'Stat.device' => 'ASC',
				'Stat.updated' => 'DESC'
			)
		)));
	}
	
	function beforeFilter() {
		parent::beforeFilter();
		
		$this->set('action', $this->params['action']);
		$this->layout = 'cracker';
	}
}
?>