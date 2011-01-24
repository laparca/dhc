<?php
print_r($test);
?>


<workunit>
	<id><?php echo $wuid ?></id>
	<algorithm><?php echo $crack->algorithm ?></algorithm>
	<charset><?php echo $crack->charset ?></charset>
$s = dbquery("SELECT * FROM `hashes` WHERE `crack` = '$cid'");
while($h = mysql_fetch_object($s))
	<?php foreach(): ?>
		<hash id='<?php echo $h->id ?>'><?php echo $h->hash ?></hash>
	<?php endforeach; ?>
	<start><?php echo $wu->start ?></start>
	<end><?php echo $wu->end ?></end>
</workunit>
