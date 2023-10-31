<?php
require_once('dbConnection.inc');
require_once('rabbitFunctions.inc');

function validateLogin($username, $password) {
    $database = dbConnect();
    $sanitizedUsername = $database->real_escape_string($username);
    $sanitizedPassword = $database->real_escape_string($password);
    $query = "SELECT * FROM Accounts WHERE Username = '$sanitizedUsername'";
    $result = $database->query($query);

    while ($row = $result->fetch_assoc()) {
        if ($sanitizedPassword === $row["PasswordHash"]) {
            
            
                return json_encode(array("returnCode" => '1', 'message' => "Logged In"));
            } else {
                return json_encode(array("returnCode" => '0', 'message' => "Error with generating session"));
            }
        
    }
    return json_encode(array("returnCode" => '0', 'message' => "No User Match The Given Username"));
}

function validateRegister($firstname, $lastname, $username, $email, $address, $city, $country, $zipcode, $password) {
    $database = dbConnect();
    $sanitizedFirstname = $database->real_escape_string($firstname);
    $sanitizedLastname = $database->real_escape_string($lastname);
    $sanitizedUsername = $database->real_escape_string($username);
    $sanitizedEmail = $database->real_escape_string($email);
    $sanitizedAddress = $database->real_escape_string($address);
    $sanitizedCity = $database->real_escape_string($city);
    $sanitizedCountry= $database->real_escape_string($country);
    $sanitizedZipcode = $database->real_escape_string($zipcode);
    $sanitizedPassword= $database->real_escape_string($password);
    $query = "SELECT * FROM Accounts WHERE Username = '$sanitizedUsername'";
    $result = $database->query($query);

    while ($row = $result->fetch_assoc()) {
        if ($row["Username"] == $sanitizedUsername) {
            return json_encode(array("returnCode" => '0', 'message' => "Username already exists, try again"));
        }
        if ($row["Email"] == $sanitizedEmail) {
            return json_encode(array("returnCode" => '0', 'message'=>"Email already exists, try again"));
        }
    }
    $insertQuery = "INSERT INTO Accounts (FirstName, LastName, Username, Email, Address, City, Country, ZipCode, PasswordHash) VALUES ('$sanitizedFirstname', '$sanitizedLastname', '$sanitizedUsername', '$sanitizedEmail', '$sanitizedAddress', '$sanitizedCity', '$sanitizedCountry', '$sanitizedZipcode', '$sanitizedPassword')";
    if ($database->query($insertQuery) === TRUE) {
        return json_encode(array("returnCode" => '1', 'message' => "Username and password accepted"));
    }
    return json_encode(array("returnCode" => '0', 'message' => "Error with registering, try again"));
}
?>
