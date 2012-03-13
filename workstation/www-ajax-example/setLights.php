<?php

// Assume that you have an eFirmata, with PWMs 3,4,5 (counting from 1) connected
// to some red, green, and blue lights.
//
// Call this as:   http://localhost/setLights.php?r=255&g=255&b=255

echo "<br><h2>Example for controlling an eFirmata from a web page.<h2><br>\n";

//eFirmata at my computer desk:
//$desk_addr = "\x00\x02\xf7\xaa\xbf\xcd";
// eFirmata on the table connected to the actual lights:
$table_addr = "\x00\x02\xf7\xaa\xff\xee";

$fileName = "/tmp/eFirmataOut.sock";

$fd = socket_create(AF_UNIX, SOCK_DGRAM, 0);
if (isset($fd) && !empty($fd))
{
    echo "Got socket.<br>";
}
socket_connect($fd, $fileName);

function minmax($val)
{
    if ($val > 255) $val = 255;
    if ($val < 0) $val = 0;
    return $val;
}

$p1 = 0;
$p2 = 0;
$red = 0;
$green = 0;
$blue = 0;
$p6 = 0;

if (isset($_GET['p1'])) $p1    = (int)$_GET['p1'];
if (isset($_GET['p2'])) $p2    = (int)$_GET['p2'];
if (isset($_GET['r']))  $red   = (int)$_GET['r'];
if (isset($_GET['g']))  $green = (int)$_GET['g'];
if (isset($_GET['b']))  $blue  = (int)$_GET['b'];
if (isset($_GET['p6'])) $p6    = (int)$_GET['p6'];

$p1 = minmax($p1);
$p2 = minmax($p2);
$red = minmax($red);
$green = minmax($green);
$blue = minmax($blue);
$p6 = minmax($p6);



$padding = "Dead Monkeys and stuff.";
$message = "S\x11 ". chr($p1) . chr($p2) . chr($red) . chr($green) . chr($blue) . chr($p6) . $padding;

$bytesSent = socket_send($fd, $table_addr . $message, 39, 0);

socket_close($fd);

echo "<br><br>";
echo "Sent " . $bytesSent . " bytes.";


