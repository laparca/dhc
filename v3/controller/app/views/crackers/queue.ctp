<table class='overview' cellspacing='0' id='ovtable'>
	<tr class='overviewheader'>
		<td>&nbsp;</td>
		<td><?php __('Alg'); ?></td>
		<td><?php __('Charset'); ?></td>
		<td><?php __('Hash'); ?></td>
		<td style='width:100px'><?php __('Max length'); ?></td>
		<td style='width:100px'><?php __('Next WU'); ?></td>
		<td style='width:150px'><?php __('Started'); ?></td>
		<td style='width:150px'><?php __('Expiration'); ?></td>
		<td><?php __('Priority'); ?></td>
		<td><?php __('Progress'); ?></td>
	</tr>

	<?php foreach($cracks as $crack): ?>
		<!-- $time = time(); -->
		<tr id='<?php echo $crack['Crack']['hash'];?>'>
			<td><?php echo $this->Html->link('[Cancel]', array('controller' => 'crackers', 'action' => 'cancel', $crack['Crack']['id'])); ?></td>';
			<td><?php echo $crack['Crack']['algorithm']; ?></td>
			<td><?php echo $crack['Crack']['charset']; ?></td>
	
			$hashes = '';
			$s = dbquery('SELECT `hash` FROM `hashes` WHERE `crack` = \'' . $row->id . '\' AND `collision` = \'\'');
			while($hash = mysql_fetch_object($s))
				$hashes .= $hash->hash . "\n";		
	
			<td><?php echo $hashes; ?></td>
			<td><?php echo $crack['Crack']['maxlen']; ?></td>
			<td><?php echo $crack['Crack']['nextwu']; ?></td>
			<td><?php echo date('Hi Ymd', $crack['Crack']['started']); ?></td>
			<td><?php echo date('Hi Ymd', $crack['Crack']['expiration']); ?></td>
			<td><?php echo $crack['Crack']['priority']; ?></td>
			<!-- 	$frac=GetStatsPercentage($row->id); -->
			<td><img src='index.php?action=ajax&page=progress&percentage=$frac' title='$frac% complete'/></td>
		</tr>
	<?php endforeach; ?>

</table>
