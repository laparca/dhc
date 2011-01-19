<!DOCTYPE html>
<html>
	<head>
		<title><?php echo $title_for_layout?></title>
		<script type="text/javascript" src="https://ajax.googleapis.com/ajax/libs/jquery/1.4.4/jquery.min.js"></script>
		<?php echo $this->Html->charset('UTF-8');?>
		<?php echo $scripts_for_layout . "\n";?>
		<link href='http://fonts.googleapis.com/css?family=Josefin+Sans:light,regular,600' rel='stylesheet' type='text/css'>
		<?php echo $this->Html->css('dhc') . "\n";?>
	</head>
	<body>
		<div id="header">
			<div id="menu">
				<ul><?php
					$menu = array(
						'overview' => 'Overview',
						'queue'    => 'Queue',
						'submit'   => 'Submit',
						'output'   => 'Output',
						'stats'    => 'Stats',
						'settings' => 'Settings'
					);
					
					foreach($menu as $menu_idx => $menu_elm): ?>
					<li <?php if($menu_idx == $action ) echo "class='active'";?>><?php echo $this->Html->link(__($menu_elm, true), array('controller' => 'crackers', 'action' => $menu_idx)); ?></li>
					<?php endforeach;?>
				<ul>
			</div>
		</div>
		<div id="content">
			<?php echo $content_for_layout?>
		</div>
		<div id="footer">
		</div>
	</body>
</html>
