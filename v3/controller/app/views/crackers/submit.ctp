<?php
echo $this->Form->create('Crackers');

echo $this->Form->input('alg', array(
	'options' => array(
		'md4'      => 'MD4',
		'md5'      => 'MD5',
		'md5crypt' => 'MD5crypt',
		'NTLM'     => 'ntlm',
		'sha1'     => 'SHA-1',
		'sha256'   => 'SHA-256'),
	'label' => __('Algorithm', true)
));

echo $this->Form->input('hash', array(
	'type'  => 'textarea',
	'label' => __('Hash Values', true)
));

$checks =  array(
	'lower' => __('Lowercase letters', true),
	'upper' => __('Uppercase letters', true),
	'nums'  => __('Numbers', true),
	'somesymbols' => __('Common symbols', true),
	'allsymbols' => __('Rare symbols', true),
	'space' => __('Space', true),
	'newline' => __('Newline', true)
);

foreach($checks as $name => $title) {
	echo $this->Form->input("$name", array(
		'type' => 'checkbox',
		'label' => "$title"
	));
}

echo $this->Form->input('len', array('label' => __('Max Length', true)));

echo $this->Form->input('exp', array(
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

echo $this->Form->input('pri', array(
	'options' => array(
		0 => __('Idle', true),
		5 => __('Normal', true),
		10 => __('Urgent', true)),
	'label' => __('Priority', true)
));

echo $this->Form->end(__('Submit', true));
?>