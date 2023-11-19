<?php
require_once('dbConnection.inc');
require_once('rabbitFunctions.inc');
require_once('SessionID.inc');


function fetchUserProfile($sessionID) {
    $sql = "SELECT Profiles.FavoriteMovie, Profiles.FavoriteGenre, Profiles.FavoriteActor, Profiles.FavoriteDirector FROM Profiles WHERE user_id = ?";
    $stmt = $pdo->prepare($sql);
    $stmt->execute([$sessionID]);
    $userPreferences = $stmt->fetch(PDO::FETCH_ASSOC);
    if (!$userPreferences) {
        echo "User preferences not found.";
    } else {
        $favoriteMovie = $userPreferences['FavoriteMovie'];
        $favoriteGenre = $userPreferences['FavoriteGenre'];
        $favoriteActor = $userPreferences['FavoriteActor'];
        $favoriteDirector = $userPreferences['FavoriteDirector'];
        $apiKey = 'ab0f6b84bbcbbd4066f4fee3eaba248c'; 
        $baseUrl = 'https://api.themoviedb.org/3';
        $genreTopMovies = getTopMovies($baseUrl, '/movie/top_rated', $apiKey, $favoriteGenre);
        $actorDirectorTopMovies = getTopMovies($baseUrl, '/discover/movie', $apiKey, $favoriteActor, $favoriteDirector);
        $criteriaTopMovies = getTopMovies($baseUrl, '/discover/movie', $apiKey, $favoriteActor, $favoriteMovie, $favoriteDirector);
        displayRecommendedMovies($genreTopMovies, 'Genre');
        displayRecommendedMovies($actorDirectorTopMovies, 'Actor and Director');
        displayRecommendedMovies($criteriaTopMovies, 'Actor, Movie, and Director');
    }
}

function getTopMovies($baseUrl, $endpoint, $apiKey, $criteria1, $criteria2 = null, $criteria3 = null) {
    $url = $baseUrl . $endpoint . '?api_key=' . $apiKey . '&with_genres=' . $criteria1;
    if ($criteria2) {
        $url .= '&with_cast=' . $criteria2 . '&with_crew=' . $criteria3;
    }
    $response = file_get_contents($url);
    $data = json_decode($response, true);
    return $data;
}

function displayRecommendedMovies($movieData, $source) {
    if ($movieData && isset($movieData['results'])) {
        $topMovies = $movieData['results'];
        $movieCount = 0;
        echo "<h3>Top 10 Recommended Movies Based on $source</h3>";
        foreach ($topMovies as $movie) {
            if ($movieCount >= 10) {
                break; 
            }
            echo $movie['title'] . ' (Rating: ' . $movie['vote_average'] . ')<br>';
            $movieCount++;
        }
    } else {
        echo "Error fetching movie recommendations based on $source.";
    }
}

function searchMoviesAndTVShows($query) {
    $apiKey = 'ab0f6b84bbcbbd4066f4fee3eaba248c';
    $url = "https://api.themoviedb.org/3/search/multi?api_key={$apiKey}&query={$query}";
    $ch = curl_init($url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    $response = curl_exec($ch);
    curl_close($ch);
    $result = json_decode($response, true);
    if (isset($result['results'])) {
        return $result['results'];
    } else {
        return null;
    }
}

function processRequest() {
    $sessionID = $_COOKIE['session_id'] ?? $_POST['session_id']; 
    if ($sessionID) {
        $userData = getUserProfileData($sessionID);
        if ($userData) {
            echo json_encode($userData);
        } else {
            echo json_encode(["message" => "User not found or an error occurred."]);
        }
    } else {
        echo json_encode(["message" => "Session ID not found in cookies or request data."]);
    }
}

processRequest();

function searchPerson($actorName) {
    $apiKey = 'ab0f6b84bbcbbd4066f4fee3eaba248c';
    $apiUrl = "https://api.themoviedb.org/3/search/person?api_key={$apiKey}&query=" . urlencode($actorName);
    $response = file_get_contents($apiUrl);
    $result = json_decode($response, true);
    if ($result && isset($result['results'][0])) {
        $person = $result['results'][0];
        $personDetails = array(
            'Name' => $person['name'],
            'ID' => $person['id'],
        );
        return $personDetails;
    } else {
        return array('error' => 'Person not found.');
    }
}



function recommendationActorDirector($username) {
    $accountId = getAccountIdByUsername($username);
    if (!$accountId) {
        return array('error' => 'Username not found.');
    }
    $conn = new mysqli(10.244.1.2, 'BackEndAdmin', 'Qg5OKQ!?$Q', 'SceneSync');
    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    }
  //update sql to match  $sql = "SELECT favorite_actor, favorite_director FROM user_preferences WHERE user_id = '$accountId'";
    $result = $conn->query($sql);
    if ($result->num_rows > 0) {
        $row = $result->fetch_assoc();
        $favoriteActor = $row['favorite_actor'];
        $favoriteDirector = $row['favorite_director'];
        $conn->close();
        $apiKey = 'ab0f6b84bbcbbd4066f4fee3eaba248c';
        $actorRecommendations = searchMoviesByPerson($favoriteActor, 'actor', $apiKey);
        $directorRecommendations = searchMoviesByPerson($favoriteDirector, 'director', $apiKey);
        return array_merge($actorRecommendations, $directorRecommendations);
    } else {
        $conn->close();
        return array('error' => 'User preferences not found.');
    }
}




function getAccountIdByUsername($username) {
    $conn = new mysqli(10.244.1.2, 'BackEndAdmin', 'Qg5OKQ!?$Q', 'SceneSync');
    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    }
    $username = $conn->real_escape_string($username);
 //update sql statement to match   $sql = "SELECT id FROM users WHERE username = '$username'";
    $result = $conn->query($sql);
    if ($result->num_rows > 0) {
        $row = $result->fetch_assoc();
        $accountId = $row['id'];
        $conn->close();
        return $accountId;
    } else {
        $conn->close();
        return false; // Username not found
    }
}

?>
