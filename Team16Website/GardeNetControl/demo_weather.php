<?php

$queryString = array();
foreach ($_POST as $key => $value) {
	$queryString[] = $value;
}
$queryString = $queryString[0];
echo $queryString;

file_put_contents('/var/www/Team16Website/garden_net/gn_util/demo_weather_reports.txt', $queryString);


?>