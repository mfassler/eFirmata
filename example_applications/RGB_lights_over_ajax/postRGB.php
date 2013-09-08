<?php

$red = (int) $_POST['r'];
$green = (int) $_POST['g'];
$blue = (int) $_POST['b'];

function minMax ($val) {
	if ($val < 0) {
		return 0;
	} elseif ($val > 255) {
		return 255;
	}
	return $val;
}

$red = minMax($red);
$green = minMax($green);
$blue = minMax($blue);


// THIS IS THE IP ADDRESS and port number for your eFirmata device:
$fp = stream_socket_client("udp://192.168.11.177:2115", $errno, $errstr);


if ($fp) {
	// For the firmata-over-UDP protocol, we must declare ourselves to be "eFirmata"
	$firmataOverUdpHeader = "eFirmata";
	// The firmata service we want: PWM
	$firmataOverUdpHeader .= "PWM";
	// Protocol version 0:
	$firmataOverUdpHeader .= "\x00";
	// No flags, no options
	$firmataOverUdpHeader .= "\x00\x00\x00\x00";

	// The firmata-over-udp header should be exactly 16 bytes:
	if (strlen($firmataOverUdpHeader) !== 16) {
		echo "BUG.  You broke it.  strlen:" . strlen($firmataOverUdpHeader) . "\n";
		exit();
	}

	$pwmMask = "\x00\x00\x00\x0e";  // we will set PWMs 1-3
	$pwmValues0 = "\x00";
	$pwmValues1_3 = chr($red) . chr($green) . chr($blue);
	$pwmValues4_31 = str_repeat("\x00", 28);

	$pwmCmd = $pwmMask . $pwmValues0 . $pwmValues1_3 . $pwmValues3_31;

	$numBytesWritten = fwrite($fp, $firmataOverUdpHeader . $pwmCmd);

	echo "In PHP:  wrote " . $numBytesWritten . " bytes to UDP (to eFirmata)";

} else {

	echo 'In PHP:  Network ERROR connecting to UDP->eFirmata';
}



