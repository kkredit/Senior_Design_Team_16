<?php

$queryString = array();
foreach ($_GET as $key => $value) {
	$queryString[] =$value;
}
$queryString = $queryString[0];


//$data = "1";
// Execute the python script with the JSON data
//$result = shell_exec('python3 /var/www/Team16Website/garden_net/gn_util/website_interface.py  ' . $queryString);
//echo $result;
file_put_contents('/var/www/Team16Website/garden_net/gn_util/ipc_file.txt', $queryString);
echo file_get_contents('/var/www/Team16Website/garden_net/gn_util/ipc_file.txt');

$address = 'localhost';
$port = 5538;

$sock = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);
$sockconnect = socket_connect($sock, $address, $port);

?>