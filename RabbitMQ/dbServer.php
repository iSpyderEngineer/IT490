#!/usr/bin/php
<?php
// Include required files for RabbitMQ connection and database-related functions
require_once('path.inc');
require_once('get_host_info.inc');
require_once('rabbitMQLib.inc');
require_once('../Backend/dbFunctions.inc');

// Function to process incoming requests
function requestProcessor($request) {
      // Print a message indicating the request received and show request details
    echo "received request" . PHP_EOL; 
    var_dump($request);

    // Switch case to handle different types of requests based on the 'type' field
    switch ($request['type']) {
        // Handle login request
        case "login":
            echo "logging in\n";
            return validateLogin($request['username'], $request['password']);

        // Handle user registration request
        case "register":
            echo "registering now\n";
            return validateRegister($request['firstname'], $request['lastname'], $request['username'], $request['email'], $request['address'], $request['city'], $request['country'], $request['zipcode'], $request['password']);

        // Handle reset password request
        case "resetPassword":
            echo "reseting password\n";
            return resetPassword($request['username'], $request['email'], $request['password']);

            // Handle validation of user preferences
        case "validatePreferences":
            echo "validating preferences\n";
            return validatePreferences($request['username'], $request['favActor'], $request['favDirector'], $request['favMovie'], $request['favGenre'], $request['biography']);
        
        // Handle request to get user profile
        case "getUserProfile":
            echo "getting user profile\n";
            return getUserProfileData($request['username']);

        // Handle request to update user profile
        case "updateProfile":
            echo "updating profile settings\n";
            return updateProfileSettings($request['username'], $request['favActor'], $request['favGenre'], $request['favDirector'], $request['favMovie'], $request['biography']);

        // Handle request to get user's watch list
        case "getWatchList":
            echo "getting watch list data\n";
            return getWatchListData($request['username']);

        // Handle request to get user's watched list
        case "getWatchedList":
            echo "getting watched list data\n";
            return getWatchedListData($request['username']);

        // Handle adding a movie to the watch list
        case "addToWatchList":
            echo "adding to watch list\n";
            return addToWatchList($request['username'], $request['MovieID'], $request['movieTitle'], $request['posterURL'], $request['year'], $request['mediaType']);

        // Handle adding a movie to the watched list
        case "addToWatchedList":
            echo "adding to watched list\n";
            return addToWatchedList($request['username'], $request['MovieID'], $request['movieTitle'], $request['posterURL'], $request['year'], $request['mediaType']);
        
        // Handle updating the user profile
        case "updateUserProfile":
            echo "updating user profile\n";
            return updateUserProfile($request['username'], $request['favActor'], $request['favMovie'], $request['favDirector'], $request['favGenres'], $request['biography']);

        // Handle search for movie reviews
        case "searchMovieReviews":
            echo "searching movie reviews\n";
            return searchRatingsByMovie($request['movieTitle']);

        // Handle request for leaderboard data
        case "getLeaderboard":
            echo "getting leaderboard\n";
            return calculateRateScoreAndLeaderboard();

        // Handle insertion of a movie review
        case "insertReview":
            echo "inserting review\n";
            return insertReview($request['username'], $request['MovieID'], $request['movieTitle'], $request['rating'], $request['review']);

        // Handle deletion of a movie from the watch list
        case "deleteFromWatchList":
            echo "deleting from watch list\n";
            return deleteFromWatchList($request['watchListID']);
        
        // Handle adding to watched list and removing from watch list
        case "addToWatchedListAndRemoveFromWatchList":
            echo "adding to watched list and removing from watch list\n";
            return addToWatchedListAndRemoveFromWatchList($request['username'], $request['MovieID'], $request['movieTitle'], $request['posterURL'], $request['year'], $request['mediaType']);
        
        // Default case for unhandled request types
        default:
            echo "Request type not handled\n";
            return ["error" => "Request type not supported"];
    }
}

// Create a new RabbitMQServer class instance with specified configuration for database access
$server = new rabbitMQServer("testRabbitMQ.ini", "database");

echo "db server started up" . PHP_EOL; // Message indicating the server has started
$server->process_requests('requestProcessor'); // Process incoming requests using the defined request processor function
echo "db server shut down" . PHP_EOL; // Message indicating the server is shutting down
exit();
?>