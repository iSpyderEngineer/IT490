<?php
require_once('dbConnection.inc');

$apiKey = 'ab0f6b84bbcbbd4066f4fee3eaba248c';

function searchMoviesandTVShows($query, $apiKey) {
    global $apiKey;
    $apiUrl = "https://api.themoviedb.org/3/search/multi?api_key={$apiKey}&query=" . urlencode($query);
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
    $conn = new mysqli('10.244.1.2', 'BackEndAdmin', 'Qg5OKQ!?$Q', 'SceneSync');
    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    }
    $sql = "SELECT FavoriteActor, FavoriteDirector FROM Profiles WHERE AccountID = '$accountId'";
    $result = $conn->query($sql);
    if ($result->num_rows > 0) {
        $row = $result->fetch_assoc();
        $favoriteActor = $row['FavoriteActor'];
        $favoriteDirector = $row['FavoriteDirector'];
        $conn->close();
        global $apiKey;
        $actorRecommendations = getMoviesByActor($favoriteActor, $apiKey);
        $directorRecommendations = getMoviesByDirector($favoriteDirector, $apiKey);
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
        if ($creditsResult && isset($creditsResult['cast'])) {
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
        if ($creditsResult && isset($creditsResult['crew'])) {
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

function getMoviesByMovieAndGenre($username) {
    global $apiKey;
    $conn = new mysqli('10.244.1.2', 'BackEndAdmin', 'Qg5OKQ!?$Q', 'SceneSync');
    if ($conn->connect_error) {
        return [];
    }
    $accountId = getAccountIdByUsername($username);
    $sql = "SELECT FavoriteGenres, FavoriteMovie FROM Profiles WHERE AccountID = '$accountId'";
    $result = $conn->query($sql);
    if ($result->num_rows > 0) {
        $profileData = $result->fetch_assoc();
        $favoriteGenre = $profileData['FavoriteGenres'];
        $favoriteMovie = $profileData['FavoriteMovie'];
        $tmdbApiUrl = 'https://api.themoviedb.org/3/';
        $tmdbDiscoverEndpoint = 'discover/movie';
        $apiEndpoint = $tmdbApiUrl . $tmdbDiscoverEndpoint . '?api_key=' . $apiKey . '&with_genres=' . urlencode($favoriteGenre);
        $apiResponse = file_get_contents($apiEndpoint);
        if ($apiResponse === false) {
            return [];
        }
        $moviesData = json_decode($apiResponse, true);
        $recommendedMovies = array_map(function ($movie) {
            return [
                'title' => $movie['title'],
                'overview' => $movie['overview'],
                'release_date' => $movie['release_date'],
            ];
        }, $moviesData['results']);
        return array_slice($recommendedMovies, 0, 10);
    } else {
        return [];
    }
}

function getAccountIdByUsername($username) {
    $conn = new mysqli('10.244.1.2', 'BackEndAdmin', 'Qg5OKQ!?$Q', 'SceneSync');
    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    }
    $username = $conn->real_escape_string($username);
    $sql = "SELECT AccountID FROM Accounts WHERE Username = '$username'";
    $result = $conn->query($sql);
    if ($result->num_rows > 0) {
        $row = $result->fetch_assoc();
        $accountId = $row['AccountID'];
        $conn->close();
        return $accountId;
    } else {
        $conn->close();
        return false;
    }
}

function getMoviesByDetails($movieId, $apiKey) {
    global $apiKey;
    $apiUrl = "https://api.themoviedb.org/3/movie/{$movieId}?api_key={$apiKey}";
    $response = file_get_contents($apiUrl);
    $result = json_decode($response, true);

    if ($result) {
        $recommendations = [
            'Title' => $result['title'],
            'PosterURL' => isset($result['poster_path']) ? "https://image.tmdb.org/t/p/w500" . $result['poster_path'] : 'Not available',
            'Genre' => [],
            'Actors' => [],
            'Director' => '',
            'ReleaseDate' => $result['release_date'],
            'Overview' => $result['overview']
        ];

        foreach ($result['genres'] as $genre) {
            $recommendations['Genre'][] = $genre['name'];
        }

        $creditUrl = "https://api.themoviedb.org/3/movie/{$movieId}/credits?api_key={$apiKey}";
        $creditResponse = file_get_contents($creditUrl);
        $creditResult = json_decode($creditResponse, true);

        if ($creditResult) {
            foreach ($creditResult['crew'] as $crewMember) {
                if (isset($crewMember['job']) && $crewMember['job'] == 'Director') {
                    $recommendations['Director'] = $crewMember['name'];
                }
            }

            foreach ($creditResult['cast'] as $castMember) {
                if (isset($castMember['known_for_department']) && $castMember['known_for_department'] == 'Acting') {
                    $recommendations['Actors'][] = $castMember['name'];
                }
            }
        } else {
            return ['error' => 'Error retrieving movie credits.'];
        }

        return $recommendations;
    } else {
        return ['error' => 'Movie not found.'];
    }
}

?>
