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

class StatsComponent extends Object {
	/**
	 * When the Stats component is initialized it sets the internal
	 * Stat and History attributes to the model. It is important to
	 * use it on each method.
	 */
	function startup(&$controller) {
		$this->Stat = ClassRegistry::init('Stat');
		$this->History = ClassRegistry::init('History');
	}
	
	/**
	 * Deletes the old stat values and the old history values.
	 */
	function deleteOld() {
		$now = time();
		$exp = $now - Configure::read('WorkUnit.expiration'); //120;
		$exp2 = $now - Configure::read('History.old'); //900;

		$this->Stat->deleteAll(array('updated <' => $exp));
		$this->History->deleteAll(array('time <' => $exp2));
	}

	/**
	 * Only deletes the old history values.
	 */
	function deleteOldHistory() {
		$exp = time() - Configure::read('History.old');
		$this->History->deleteAll(array('time <' => $exp));
	}
	
	/**
	 * Check the stat values to calculate the history.
	 */
	function updateHistory() {
		//See if we already have history for this timestamp
		//TODO: Make this more efficient
		$now = time();
		
		$r = $this->History->find('first', array(
			'conditions' => array('time' => $now)
		));
		if(empty($r))
		{
			$fspeed = 0;
			$stats = $this->Stat->find('list',
				array(
					'fields' => array('Stat.id', 'Stat.speed')
				)
			);
			foreach($stats as $id => $speed) {
				$fspeed += $speed;
			}

			//Add to global stats
			$this->History->save(
				array(
					'History' => array(
						'time' => $now,
						'speed' => $fspeed
					)
				)
			);
		}
	}
	
	/**
	 * Return the whole history
	 */
	function getHistory() {
		return $this->History->find('all', array('order' => 'History.time ASC'));
	}
	
	function addStat($stat) {
		$this->Stat->deleteAll(array('device' => $stat['device']));
		//Add us to stats
		$this->Stat->save(array('Stat' => $stat));
	}
}
?>