<?php

$queryString = array();
foreach ($_POST as $key => $value) {
	$queryString[] = $value;
}
$queryString = $queryString[0];
echo $queryString;
file_put_contents('/var/www/Team16Website/garden_net/gn_util/user_info.txt', $queryString);

$address = 'localhost';
$port = 5537;

$sock = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
$sockconnect = socket_connect($sock, $address, $port);


?>