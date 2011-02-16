<?php
Configure::write('debug', 0);
for($i = 0; $i < count($history); $i ++) {
	$history[$i]['History']['time'] = date('Y-m-d H:i:s', $history[$i]['History']['time']);
}
echo $this->Javascript->object($history);
?> 
