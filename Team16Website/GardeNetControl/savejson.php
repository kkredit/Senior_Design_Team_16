<!doctype html>
<html>
<head>
<meta charset="utf-8">
<title>Untitled Document</title>
</head>

<body>

<?php
	
	$thefile = "new.json";
	chmod($thefile, 0774);
	$towrite = $_POST['checks'];
	echo $towrite;
	$openedfile = fopen($thefile, "w");
	$encoded = json_encode($towrite);
	fwrite($openedfile, $encoded);
	fclose($openedfile);
	return "<br> <br>".$towrite;


?>



</body>
</html>
