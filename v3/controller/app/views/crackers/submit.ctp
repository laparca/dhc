<?php
echo $this->Session->flash();

function errorMsg($msg = '', $html) {
	return (empty($msg)?'':$html->tag('span', $msg, array('class' => 'error')));
}
echo $this->Form->create('Cracker');

echo $this->Form->input('alg', array(
	'label' => __('Algorithm', true),
	'after' => @errorMsg($algError, $this->Html),
	'options' => array(
		'md4'      => 'MD4',
		'md5'      => 'MD5',
		'md5crypt' => 'MD5crypt',
		'NTLM'     => 'ntlm',
		'sha1'     => 'SHA-1',
		'sha256'   => 'SHA-256')
));

echo $this->Form->input('hashes', array(
	'type'  => 'textarea',
	'label' => __('Hash Values', true),
	'after' => @errorMsg($hashesError, $this->Html)
));

echo $this->Form->input('chset', array(
	'type' => 'select',
	'multiple' => 'checkbox',
	'label' => __('Character set', true),
	'options' => array(
		'lower' => __('Lowercase letters', true),
		'upper' => __('Uppercase letters', true),
		'nums'  => __('Numbers', true),
		'somesymbols' => __('Common symbols', true),
		'allsymbols' => __('Rare symbols', true),
		'space' => __('Space', true),
		'newline' => __('Newline', true)
	)
));

echo $this->Form->input('len', array(
	'label' => __('Max Length', true),
	'after' => @errorMsg($lenError)
));

echo $this->Form->input('exp', array(
	'label' => __('Expiration', true),
	'after' => @errorMsg($expError),
	'options' => array(
		15 => __('15 minutes', true),
		30 => __('30 minutes', true),
		60 => __('1 hour', true),
		120 => __('2 hours', true),
		300 => __('5 hours', true),
		720 => __('12 hours', true),
		1440 => __('1 day', true),
		10080 => __('1 week', true),
		0 => __('Never', true))
));

echo $this->Form->input('pri', array(
	'label' => __('Priority', true),
	'after' => @errorMsg($priError),
	'options' => array(
		0 => __('Idle', true),
		5 => __('Normal', true),
		10 => __('Urgent', true))
));

echo $this->Form->end(__('Submit', true));
?>