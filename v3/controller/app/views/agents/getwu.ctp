<?php
//print_r($test);
//print_r($info);
?>
<workunit>
	<id><?php echo $info['id'] ?></id>
	<algorithm><?php echo $info['algorithm'] ?></algorithm>
	<charset><?php echo $info['charset'] ?></charset>
	<?php foreach($info['hashes'] as $idx => $hash): ?>
		<?php if(is_array($hash)) : ?>
			<hash id='<?php echo $hash['id'] ?>'><?php echo $hash['hash'] ?></hash>
		<?php else : ?>
			<hash id='<?php echo $idx ?>'><?php echo $hash ?></hash>
		<?php endif; ?>
	<?php endforeach; ?>
	<start><?php echo $info['start'] ?></start>
	<end><?php echo $info['end'] ?></end>
</workunit>
