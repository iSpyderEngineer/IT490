<?php
require_once('dbConnection.inc');

$apiKey = 'ab0f6b84bbcbbd4066f4fee3eaba248c';

/*
function searchMoviesandTVShows($query) {
    global $apiKey;
    $apiUrl = "https://api.themoviedb.org/3/search/multi?api_key={$apiKey}&query=" . urlencode($query);
    $response = file_get_contents($apiUrl);
    $result = json_decode($response, true);
    // If the results were true it will will start extracting the relevant data 
    if ($result && isset($result['results'])) {
        $mediaList = array();
        foreach ($result['results'] as $media) {
            $mediaType = '';
            if (isset($media['media_type'])) {
                $mediaType = $media['media_type'];
            } elseif (isset($media['original_title'])) {
                $mediaType = 'movie';
            } elseif (isset($media['original_name'])) {
                $mediaType = 'tvshow';
            }

        // Will check the media type whether its a movie or tv show 
        foreach ($result['results'] as $media) {
            $mediaList[] = array(
                'ID' => $media['id'],
                'Title' => $media['original_title'] ?? $media['original_name'],
                'Type' => $media['media_type'],
                'PosterURL' => isset($media['poster_path']) ? "https://image.tmdb.org/t/p/w500{$media['poster_path']}" : 'Not available',
                'Type' => $mediaType);
        }
        return $mediaList;
    } else {
        return array('error' => 'No results found.');
    }
}
*/

function searchMoviesAndTVShows($query) {
    global $apiKey;
    $apiUrl = "https://api.themoviedb.org/3/search/multi?api_key={$apiKey}&query=" . urlencode($query);
    $response = file_get_contents($apiUrl);
    $result = json_decode($response, true);
    if ($result && isset($result['results'])) {
        $mediaList = array();
        foreach ($result['results'] as $media) {
            $mediaType = '';
            if (isset($media['media_type'])) {
                $mediaType = $media['media_type'];
            } elseif (isset($media['original_title'])) {
                $mediaType = 'movie';
            } elseif (isset($media['original_name'])) {
                $mediaType = 'tvshow';
            }
            $mediaList[] = array(
                'ID' => $media['id'],
                'Title' => $media['original_title'] ?? $media['original_name'],
                'Type' => $mediaType,
                'PosterURL' => isset($media['poster_path']) ? "https://image.tmdb.org/t/p/w500{$media['poster_path']}" : 'Not available',
            );
        }
        return $mediaList;
    } else {
        return array('error' => 'No results found.');
    }
}

function searchPerson($actorName) {
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
        $actorRecommendations = getMoviesByActor($favoriteActor);
        $directorRecommendations = getMoviesByDirector($favoriteDirector);
        return array_merge($actorRecommendations, $directorRecommendations);
    } else {
        $conn->close();
        return array('error' => 'User preferences not found.');
    }
}

function getMoviesByActor($favoriteActor) {
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

function getMoviesByDirector($favoriteDirector) {
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

function getMoviesByDetails($movieId) {
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



//isfar
function getRecentWatchedRecommendations($username) {
    $accountId = getAccountIdByUsername($username);
    if (!$accountId) {
        return array('error' => 'Username not found.');
    }
    $mostRecentWatched = getMostRecentWatched($username);
    if (!$mostRecentWatched) {
        return getPopularRecommendations($apiKey);
    }
    $recentTitle = $mostRecentWatched['MovieTitle'];
    $recentPosterPath = $mostRecentWatched['PosterURL'];
    $recommendations = searchMoviesByTitle($recentTitle);
    $recommendations += getPopularRecommendations($apiKey, 10 - count($recommendations));
    return $recommendations;
}
//isfar
function searchMoviesByTitle($title) {
    global $apiKey;
    $url = "https://api.themoviedb.org/3/search/movie";
    $params = [
        'api_key' => $apiKey,
        'query' => urlencode($title),
    ];
    $url .= '?' . http_build_query($params);
    $ch = curl_init($url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    $response = curl_exec($ch);
    curl_close($ch);
    $data = json_decode($response, true);
    if (isset($data['results'])) {
        $movies = [];
        foreach ($data['results'] as $result) {
            $movies[] = [
                'title' => $result['title'],
                'poster_path' => $result['poster_path'],
                'release_year' => $result['release_date'],
            ];
        }
        return $movies;
    } else {
        return [];
    }
}

//isfar
function getMostRecentWatched($username) {
    global $apiKey;
    $accountId = getAccountIdByUsername($username);
    if (!$accountId) {
        return array('error' => 'Username not found.');
    }
    $conn = new mysqli('10.244.1.2', 'BackEndAdmin', 'Qg5OKQ!?$Q', 'SceneSync');
    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    }
    $sql = "SELECT Movie, PosterURL, Year
            FROM WatchedList
            WHERE AccountID = ?
            LIMIT 1";
    $stmt = $conn->prepare($sql);
    $stmt->bind_param("i", $accountId);
    $stmt->execute();
    $result = $stmt->get_result();
    if ($row = $result->fetch_assoc()) {
        $mostRecentWatched = array(
            'MovieTitle' => $row['Movie'],
            'PosterURL' => $row['PosterURL'],
            'Year' => $row['Year'],
        );
    } else {
        $mostRecentWatched = null;
    }
    $stmt->close();
    $conn->close();
    return $mostRecentWatched;
}
function getTVByDetails($movieId) {
    global $apiKey;
    $apiUrl = "https://api.themoviedb.org/3/tv/{$movieId}?api_key={$apiKey}";
    $response = file_get_contents($apiUrl);
    $result = json_decode($response, true);

    if ($result) {
        $recommendations = [
            'name' => $result['name'],
            'PosterURL' => isset($result['poster_path']) ? "https://image.tmdb.org/t/p/w500" . $result['poster_path'] : 'Not available',
            'Genre' => [],
            'Actors' => [],
            'Director' => '',
            'first_air_date' => $result['first_air_date'],
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
