<table class='output' cellspacing='0'>
	<thead>
		<tr class='outputheader'>
			<th></th>
			<th><?php __('Alg') ?></th>
			<th><?php __('Charset') ?></th>
			<th><?php __('Hash') ?></th>
			<th>State</th>
			<th>Max length</th>
			<th>Value</th>
		</tr>
	</thead>

	<tbody>
		<?php foreach($hashes as $hash): ?>
			<tr>
				<td></td><!--TODO: add command links-->
				<td><?php echo $hash['Crack']['algorithm']; ?></td>
				<td><?php echo $hash['Crack']['charset']; ?></td>
				<td><?php echo $hash['Hash']['hash']; ?></td>
				<td><?php ($hash['Hash']['collision'] != '' ? __('Found') : ($hash['Crack']['updated'] > $hash['Crack']['expiration'] ? __('Time expired') : __('Search space exhausted'))); ?></td>
				<td><?php echo $hash['Crack']['maxlen']; ?></td>
				<td><?php echo $hash['Hash']['collision']; ?></td>
			</tr>
		<?php endforeach; ?>
	</tbody>
</table>