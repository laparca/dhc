<?php
function endHostname()
{
	global $hostname;
	global $tspeed;
	global $speeds;
	global $devs;
	global $dts;
	
	if($hostname != null) : ?>
			<td><?php echo $devs . __('Total'); ?></td>
			<td><?php echo $speeds . $tspeed; ?></td>
			<td><?php echo $dts ?></td>		
		</tr>
		<?php $devs = ''; ?>
		<?php $speeds = ''; ?>
		<?php $dts = ''; ?>
		<?php $tspeed  = ''; ?>
	<?php endif;
}
?>
<table class='overview' cellspacing='0'>
	<thead>
		<tr class='overviewheader'>
			<th>Hostname</th>
			<th>Devices</th>
			<th>Speed (MHashes/sec)</th>
			<th>Last activity</th>
		</tr>
	</thead>
	
	<tbody>

		<?php	$hostname = null; ?>

		<?php $devs = ''; ?>
		<?php $speeds = ''; ?>
		<?php $dts = ''; ?>
		<?php $tspeed  = ''; ?>
		<?php $now = time(); ?>

		<?php foreach($stats as $stat) : ?>
			<?php
			$desc = explode('-', $stats['Stat']['device']);
			$stat['Stat']['hostname'] = $desc[0];
			$stat['Stat']['dev'] = $desc[1] . ' ' . $desc[2];
			?>
			<?php if($stat['Stat']['hostname'] != $hostname) : ?>

				<?php endHostname(); ?>

				<tr style='vertical-align:top'>
					<td><?php echo $stat['Stat']['hostname'] ?></td>

				<?php $hostname = $stat['Stat']['hostname'];	?>

			<?php endif; ?>

			<?php $devs .= $stat['Stat']['dev'] . '<br/>'; ?>
			<?php $speeds .= $stat['Stat']['speed'] . '<br/>'; ?>
			<?php $tspeed += $stat['Stat']['speed']; ?>
			<?php $dts .= ($noe - $stat['Stat']['updated']) . ' seconds ago<br/>'; ?>


		<?php endforeach; ?>

		<?php endHostname(); ?>

	</tbody>
</table>

<?php
// $device = hostname-type-num
//	if($old_hostname != $hostname) {
//		Distinto hostname
//	}
//	else if($old_type_num != $type_num) { <-- Is always different
//		Other speed & dt
//	}
	/*
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
*/
?>