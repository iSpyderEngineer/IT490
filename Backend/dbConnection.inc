<?php
function dbConnect() {
	$login = new mysqli("10.244.1.2","BackEndAdmin","Qg5OKQ!?$Q","SceneSync");

	if ($login -> connect_errno != 0) {
		echo "Error connecting to database: " . $this -> login -> connect_error . PHP_EOL;
		exit(1);
	}
	echo "correctly connected to database" . PHP_EOL;
	return $login;
}
?>
