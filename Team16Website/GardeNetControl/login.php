<?php
$queryString = array();
foreach ($_POST as $key => $value) {
	$queryString[] = $value;
}
$queryString = $queryString[0];
$obj = json_decode($queryString);
$user = $obj->{'Username1'};
$pass = $obj->{'Password1'};
$directory = escapeshellarg("/etc/apache2/.htpasswd");
exec("rm ".$directory);
$command = "htpasswd -cb ".$directory." ".$user." ".$pass;
exec($command, $output);
echo "Password changed!\nNote: You may have to sign in again.";


?>