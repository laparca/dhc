<h1><?php __('Queue')?></h1>
<table class='overview' cellspacing='0' id='ovtable'>
	<thead>
		<tr class='overviewheader'>
			<th class="ov_col01">&nbsp;</th>
			<th class="ov_col02"><?php __('Alg'); ?></th>
			<th class="ov_col03"><?php __('Charset'); ?></th>
			<th class="ov_col04"><?php __('Hash'); ?></th>
			<th class="ov_col05" style='width:100px'><?php __('Max length'); ?></th>
			<th class="ov_col06" style='width:100px'><?php __('Next WU'); ?></th>
			<th class="ov_col07" style='width:150px'><?php __('Started'); ?></th>
			<th class="ov_col08" style='width:150px'><?php __('Expiration'); ?></th>
			<th class="ov_col09"><?php __('Priority'); ?></th>
			<th class="ov_col10"><?php __('Progress'); ?></th>
		</tr>
	</thead>

	<tbody>
		<?php foreach($cracks as $crack): ?>
			<!-- $time = time(); -->
			<tr id='<?php echo $crack['Crack']['hash'];?>'>
				<td class="ov_col01"><?php echo $this->Html->link('[Cancel]', array('controller' => 'crackers', 'action' => 'cancel', $crack['Crack']['id'])); ?></td>
				<td class="ov_col02"><?php echo $crack['Crack']['algorithm']; ?></td>
				<td class="ov_col03"><?php echo $crack['Crack']['charset']; ?></td>
				<td class="ov_col04">
					<?php foreach($crack['Hash'] as $hash): ?>
						<p><?php echo $hash['hash']; ?></p>
					<?php endforeach; ?>
				</td>
				<td class="ov_col05"><?php echo $crack['Crack']['maxlen']; ?></td>
				<td class="ov_col06"><?php echo $crack['Crack']['nextwu']; ?></td>
				<td class="ov_col07"><?php echo date('d/m/Y H:i', $crack['Crack']['started']); ?></td>
				<td class="ov_col08"><?php echo date('d/m/Y H:i', $crack['Crack']['expiration']); ?></td>
				<td class="ov_col09"><?php echo $crack['Crack']['priority']; ?></td>
				<!-- 	$frac=GetStatsPercentage($row->id); -->
				<td class="ov_col10"><img src='index.php?action=ajax&page=progress&percentage=$frac' title='$frac% complete'/></td>
			</tr>
		<?php endforeach; ?>
	</tbody>
</table>