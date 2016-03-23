<?php

$queryString = array();
foreach ($_GET as $key => $value) {
	$queryString[] =$value;
}
$queryString = implode('&', $queryString);


//$data = "1";
// Execute the python script with the JSON data
$result = shell_exec('python /var/www/Team16Website/printjson.py  ' . $queryString);
echo $result;
?>