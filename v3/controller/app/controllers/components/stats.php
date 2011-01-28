<?php
class StatsComponent extends Object {
	function startup(&$controller) {
		$this->Stat = ClassRegistry::init('Stat');
		$this->History = ClassRegistry::init('History');
	}
	
	function deleteOld() {
		$now = time();
		$exp = $now - 120;
		$exp2 = $now - 900;

		$this->Stat->deleteAll(array('updated <' => $exp));
		$this->History->deleteAll(array('time <' => $exp2));
	}

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

	function addStat($stat) {
		$this->Stat->deleteAll(array('device' => $stat['device']));
		//Add us to stats
		$this->Stat->save(array('Stat' => $stat));
	}
}
?>