<?php
$queryString = array();
foreach ($_POST as $key => $value) {
	$queryString[] = $value;
}
$queryString = $queryString[0];
echo $queryString;

file_put_contents('/var/www/Team16Website/garden_net/gn_util/garden_power_status.txt', $queryString);
//echo file_get_contents('/var/www/Team16Website/garden_net/gn_util/ipc_file.txt');

$address = 'localhost';
$port = 5535;

$sock = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
$sockconnect = socket_connect($sock, $address, $port);


?>
