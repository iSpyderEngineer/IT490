<?php
require_once('dbConnection.inc');
require_once('rabbitFunctions.inc');
require_once('SessionID.inc');
$apiKey = 'ab0f6b84bbcbbd4066f4fee3eaba248c';




function searchMoviesandTVShows($query, $apiKey) {
    $apiUrl = "https://api.themoviedb.org/3/search/multi?api_key={$apiKey}&query=" . urlencode($query);
    global $apiKey;
    $response = file_get_contents($apiUrl);
    $result = json_decode($response, true);
    if ($result && isset($result['results'])) {
        $mediaList = array();
        foreach ($result['results'] as $media) {
            $mediaList[] = array(
                'Title' => $media['original_title'] ?? $media['original_name'],
                'Type' => $media['media_type'],
                'PosterURL' => isset($media['poster_path']) ? "https://image.tmdb.org/t/p/w500{$media['poster_path']}" : 'Not available',
            );
        }
        return $mediaList;
    } else {
        return array('error' => 'No results found.');
    }
}



function searchPerson($actorName, $apiKey) {
    global $apiKey;
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
  //  update sql to match  $sql = "SELECT favorite_actor, favorite_director FROM user_preferences WHERE user_id = '$accountId'";
    $result = $conn->query($sql);
    if ($result->num_rows > 0) {
        $row = $result->fetch_assoc();
        $favoriteActor = $row['favorite_actor'];
        $favoriteDirector = $row['favorite_director'];
        $conn->close();
        global $apiKey;
        $actorRecommendations = searchMoviesByPerson($favoriteActor, 'actor', $apiKey);
        $directorRecommendations = searchMoviesByPerson($favoriteDirector, 'director', $apiKey);
        return array_merge($actorRecommendations, $directorRecommendations);
    } else {
        $conn->close();
        return array('error' => 'User preferences not found.');
    }
}

function getMoviesByActor($favoriteActor, $apiKey) {
    global $apiKey;
    $apiUrl = "https://api.themoviedb.org/3/search/person?api_key={$apiKey}&query=" . urlencode($favoriteActor);
    $response = file_get_contents($apiUrl);
    $result = json_decode($response, true);
    if ($result && isset($result['results'][0])) {
        $personId = $result['results'][0]['id'];
        $creditsUrl = "https://api.themoviedb.org/3/person/{$personId}/movie_credits?api_key={$apiKey}";
        $creditsResponse = file_get_contents($creditsUrl);
        $creditsResult = json_decode($creditsResponse, true);
        if ($creditsResult) {
            $recommendations = array();
            foreach ($creditsResult['cast'] as $movie) {
                $recommendations[] = array(
                    'Title' => $movie['title'],
                    'PosterURL' => isset($movie['poster_path']) ? "https://image.tmdb.org/t/p/w500{$movie['poster_path']}" : 'Not available',
                );
            }
            return $recommendations;
        } else {
            return array('error' => 'Error retrieving movie credits.');
        }
    } else {
        return array('error' => 'Actor not found.');
    }
}

function getMoviesByDirector($favoriteDirector, $apiKey) {
    global $apiKey;
    $apiUrl = "https://api.themoviedb.org/3/search/person?api_key={$apiKey}&query=" . urlencode($favoriteDirector);
    $response = file_get_contents($apiUrl);
    $result = json_decode($response, true);
    if ($result && isset($result['results'][0])) {
        $personId = $result['results'][0]['id'];
        $creditsUrl = "https://api.themoviedb.org/3/person/{$personId}/movie_credits?api_key={$apiKey}";
        $creditsResponse = file_get_contents($creditsUrl);
        $creditsResult = json_decode($creditsResponse, true);
        if ($creditsResult) {
            $recommendations = array();
            foreach ($creditsResult['crew'] as $movie) {
                if (isset($movie['job']) && $movie['job'] == 'Director') {
                    $recommendations[] = array(
                        'Title' => $movie['title'],
                        'PosterURL' => isset($movie['poster_path']) ? "https://image.tmdb.org/t/p/w500{$movie['poster_path']}" : 'Not available',
                    );
                }
            }
            return $recommendations;
        } else {
            return array('error' => 'Error retrieving movie credits.');
        }
    } else {
        return array('error' => 'Director not found.');
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
        return false; 
    }
}

?>
