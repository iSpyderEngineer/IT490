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
?>
