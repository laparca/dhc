<h1><?php __('Stats')?></h1>
<?php
class StatsHelper {
	var $hostname = null;

	var $devs = '';
	var $speeds = '';
	var $dts = '';
	var $tspeed  = '';
	var $now = null;
	
	function __construct() {
		$this->now = time();
	}

	function endHostname()
	{
		if($this->hostname != null) : ?>
			<td><?php echo $this->devs . __('Total', true); ?></td>
			<td><?php echo $this->speeds . $this->tspeed; ?></td>
			<td><?php echo $this->dts ?></td>
			</tr>
			<?php $this->devs = ''; ?>
			<?php $this->speeds = ''; ?>
			<?php $this->dts = ''; ?>
			<?php $this->tspeed  = ''; ?>
		<?php endif;
	}

	function updateValues($stat) {
		$this->devs .= $stat['Stat']['dev'] . '<br/>';
		$this->speeds .= $stat['Stat']['speed'] . '<br/>';
		$this->tspeed += $stat['Stat']['speed'];
		$this->dts .= ($this->now - (int)$stat['Stat']['updated']) . ' ' .__('seconds ago', true) .'<br/>';
	}

	function setHostname($hostname) {
		$this->hostname = $hostname;
	}

	function getHostname() {
		return $this->hostname;
	}

}

$sth = new StatsHelper();
?>
<!--
<?php print_r($stats);?>
-->
<table class='overview' cellspacing='0'>
	<thead>
		<tr class='overviewheader'>
			<th><?php __('Hostname'); ?></th>
			<th><?php __('Devices'); ?></th>
			<th><?php __('Speed (MHashes/sec)'); ?></th>
			<th><?php __('Last activity'); ?></th>
		</tr>
	</thead>
	
	<tbody>
		<?php foreach($stats as $stat) : ?>
			<?php
			$desc = explode('-', $stat['Stat']['device']);
			$stat['Stat']['hostname'] = $desc[0];
			$stat['Stat']['dev'] = $desc[1] . ' ' . $desc[2];
			?>
			<?php if($stat['Stat']['hostname'] != $sth->getHostname()) : ?>
				<?php $sth->endHostname(); ?>

				<tr style='vertical-align:top'>
					<td><?php echo $stat['Stat']['hostname'] ?></td>

				<?php $sth->setHostname($stat['Stat']['hostname']);	?>

			<?php endif; ?>

			<?php $sth->updateValues($stat); ?>

		<?php endforeach; ?>

		<?php $sth->endHostname(); ?>

	</tbody>
</table>