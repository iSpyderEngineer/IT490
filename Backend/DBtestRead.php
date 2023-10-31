<?php
// Database connection
$host = '10.244.1.2';
$username = 'BackEndAdmin';
$password = 'Qg5OKQ!?$Q';
$database = 'SceneSync';

$conn = new mysqli($host, $username, $password, $database);

if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

//$sql = "INSERT INTO Accounts (firstName, LastName, Username, Email, Address, City, Country, ZipCode, PasswordHash) VALUES ('Tester', 'One', 'test1', 'test1@testing.com', '801 Testing Street', 'Test City', 'USA', '12345', 'abcdefghijklmnop')";

$sql = "SELECT * FROM Accounts";
$result = $conn->query($sql);


if ($result->num_rows > 0) {
    // output data of each row
    while($row = $result->fetch_assoc()) {
      echo "AccountId: " . $row["AccountID"]. " -Name: " . $row["FirstName"]. " " . $row["LastName"]. " " . $row["Username"]. " " . $row["Email"]. " " . $row["Address"]. " " . $row["City"]. " " . $row["Country"]. " " . $row["ZipCode"]. " " . $row["PasswordHash"]. " ";
    }

  } else {
    echo "0 results";
  }


  $sql2 = "SELECT AccountID, Username, PasswordHash FROM Accounts";
  $results2 = $conn->query($sql2);
  $x = 0;
  if($results2->num_rows > 0) {
    while($row = $results2->fetch_assoc()) {
     echo "id: " . $row["AccountID"]. " Username: " . $row["Username"]. " PasswordHash: " . $row["PasswordHash"]. "</n>";
    $x = $x+1;
    }
  } else {
    echo "0 results";
  }
  $conn->close();
  

/*if ($conn->query($sql) ===TRUE) {
    echo "New record created successfully";
} else {
    echo "Error: " . $sql . "<br>" . $conn->error;
}

$conn->close();
*/

?>