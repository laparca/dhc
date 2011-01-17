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

echo $this->Form->end(__('Submit', true));
?>