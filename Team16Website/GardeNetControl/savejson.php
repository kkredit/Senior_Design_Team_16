<!doctype html>
<html>
<head>
<meta charset="utf-8">
<title>Untitled Document</title>
</head>

<body>

<?php
	
	$thefile = "/usr/local/apache/htdocs/academic/engr/2015-16-team16/GardeNetControl/new.json";
	chmod($thefile, 0774);
	$towrite = $_POST["myForm"];
	echo $towrite;
	$openedfile = fopen($thefile, "w");
	$encoded = json_encode($towrite);
	fwrite($openedfile, $encoded);
	fclose($openedfile);
	return "<br> <br>".$towrite;


?>



</body>
</html>