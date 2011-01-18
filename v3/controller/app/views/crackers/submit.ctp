<?php
echo $this->Form->create('Crackers');

echo $this->Form->input('Algorithm', array(
	'options' => array(
		'md4'      => 'MD4',
		'md5'      => 'MD5',
		'md5crypt' => 'MD5crypt',
		'NTLM'     => 'ntlm',
		'sha1'     => 'SHA-1',
		'sha256'   => 'SHA-256'),
	'label' => __('Algorithm', true)
));

echo $this->Form->input('Hash Values', array(
	'type'  => 'textarea',
	'label' => __('Hash Values', true)
));

echo $this->Form->input('Max Length', array('label' => __('Max Length', true)));

echo $this->Form->input('Expiration', array(
	'options' => array(
		15 => __('15 minutes', true),
		30 => __('30 minutes', true),
		60 => __('1 hour', true),
		120 => __('2 hours', true),
		300 => __('5 hours', true),
		720 => __('12 hours', true),
		1440 => __('1 day', true),
		10080 => __('1 week', true),
		0 => __('Never', true)),
	'label' => __('Expiration', true)
));

echo $this->Form->input('Priority', array(
	'options' => array(
		0 => __('Idle', true),
		5 => __('Normal', true),
		10 => __('Urgent', true)),
	'label' => __('Priority', true)
));

echo $this->Form->end(__('Submit', true));
?>