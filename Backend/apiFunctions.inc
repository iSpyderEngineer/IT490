<?php
require_once('dbConnection.inc');

$apiKey = 'ab0f6b84bbcbbd4066f4fee3eaba248c';


// This function takes one parmater $query
function searchMoviesAndTVShows($query) {
    // Key is used for connecting to the TMBD
    global $apiKey;
    $apiUrl = "https://api.themoviedb.org/3/search/multi?api_key={$apiKey}&query=" . urlencode($query);
    // Json response is put into the $result 
    $response = file_get_contents($apiUrl);
    $result = json_decode($response, true);
    // If the results were true it will will start extracting the relevant data 
    if ($result && isset($result['results'])) {
        $mediaList = array();
        // Will check the media type whether its a movie or tv show 
        foreach ($result['results'] as $media) {
            $mediaType = '';
            if (isset($media['media_type'])) {
                $mediaType = $media['media_type'];
            } elseif (isset($media['original_title'])) {
                $mediaType = 'movie';
            } elseif (isset($media['original_name'])) {
                $mediaType = 'tvshow';
            }
            // Gets the ID, Title, Type, and poster url 
            $mediaList[] = array(
                'ID' => $media['id'],
                'Title' => $media['original_title'] ?? $media['original_name'],
                'Type' => $mediaType,
                'PosterURL' => isset($media['poster_path']) ? "https://image.tmdb.org/t/p/w500{$media['poster_path']}" : 'Not available',
            );
        }
        // Returns the mediaList with relevant information 
        return $mediaList;
    } else {
        return array('error' => 'No results found.');
    }
}
// This function takes the parameter $actorName 
function searchPerson($actorName) {
    // API key for the movie database
    global $apiKey;
    // The API Url for finding the particular person 
    $apiUrl = "https://api.themoviedb.org/3/search/person?api_key={$apiKey}&query=" . urlencode($actorName);
    // A get request is made to the movie database
    $response = file_get_contents($apiUrl);
    // Json response is decoded 
    $result = json_decode($response, true);
    // Checks if there was atleast one person succesfully found
    if ($result && isset($result['results'][0])) {
        $person = $result['results'][0];
    // If there was one person found it will return the name of the Person and ID
        $personDetails = array(
            'Name' => $person['name'],
            'ID' => $person['id'],
        );
        return $personDetails;
    } else {
        return array('error' => 'Person not found.');
    }
}
// This function takes the parameter $username. Recomendations will be given depending on your favorite actor and director 
function recommendationActorDirector($username) {
    // Gets the Account ID associated with a particular user name.
    $accountId = getAccountIdByUsername($username);
    // If the user name is not found it will return an error message "Username not found" 
    if (!$accountId) {
        return array('error' => 'Username not found.');
    }
    // This will connect to our database using our credentials. If the connection fails it will display a message 
    $conn = new mysqli('10.244.1.2', 'BackEndAdmin', 'Qg5OKQ!?$Q', 'SceneSync');
    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    }
    // This selects the favorite and actor with the associated account ID
    $sql = "SELECT FavoriteActor, FavoriteDirector FROM Profiles WHERE AccountID = '$accountId'";
    $result = $conn->query($sql);
    // if there is something there it will then proceed to extract the relevant info
    if ($result->num_rows > 0) {
        $row = $result->fetch_assoc();
        $favoriteActor = $row['FavoriteActor'];
        $favoriteDirector = $row['FavoriteDirector'];
        $conn->close();
        global $apiKey;
        // It than gets the favorite actor and director for the recomendations to be given based on the user's prefrence 
        $actorRecommendations = getMoviesByActor($favoriteActor);
        $directorRecommendations = getMoviesByDirector($favoriteDirector);
        return array_merge($actorRecommendations, $directorRecommendations);
    // if nothing was found an error message is shown 
    } else {
        $conn->close();
        return array('error' => 'User preferences not found.');
    }
}
// This function takes one parameter $favoriteActor 
function getMoviesByActor($favoriteActor) {
    // Key used to connect to the movie database
    global $apiKey;
    // The actor's name is put in the API url  
    $apiUrl = "https://api.themoviedb.org/3/search/person?api_key={$apiKey}&query=" . urlencode($favoriteActor);
    // A get request is made to the movie database the json response is held in $result
    $response = file_get_contents($apiUrl);
    $result = json_decode($response, true);
    // This if statement will check if their was atleast one person. The person ID is taken. 
    if ($result && isset($result['results'][0])) {
        $personId = $result['results'][0]['id'];
        $creditsUrl = "https://api.themoviedb.org/3/person/{$personId}/movie_credits?api_key={$apiKey}";
        // Movie credits are found for the actor 
        $creditsResponse = file_get_contents($creditsUrl);
        $creditsResult = json_decode($creditsResponse, true);
        // Goes through the cast and gets the credits 
        if ($creditsResult && isset($creditsResult['cast'])) {
            $recommendations = array();
            foreach ($creditsResult['cast'] as $movie) {
            // Title and Poster will be put in an array for the cast. If nobody is found an error message will be displayed and if succesful the recomendations will be displayed
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
// This function takes the parameter $favoriteDirector  
function getMoviesByDirector($favoriteDirector) {
    // Connects to the movie database
    global $apiKey;
    $apiUrl = "https://api.themoviedb.org/3/search/person?api_key={$apiKey}&query=" . urlencode($favoriteDirector);
    // A get request is made to the movie database 
    $response = file_get_contents($apiUrl);
    // Json is than decoded in $result 
    $result = json_decode($response, true);
    // Checks to see if there was atleast one person it then checks for the person ID
    if ($result && isset($result['results'][0])) {
        $personId = $result['results'][0]['id'];
        // Another get request is made to get the credits for the director
        $creditsUrl = "https://api.themoviedb.org/3/person/{$personId}/movie_credits?api_key={$apiKey}";
        $creditsResponse = file_get_contents($creditsUrl);
        $creditsResult = json_decode($creditsResponse, true);
        // In the cast it will check for directors 
        if ($creditsResult && isset($creditsResult['crew'])) {
            $recommendations = array();
            foreach ($creditsResult['crew'] as $movie) {
                // if they are a director it will display reccomendations and will display the title and poster url 
                if (isset($movie['job']) && $movie['job'] == 'Director') {
                    $recommendations[] = array(
                        'Title' => $movie['title'],
                        'PosterURL' => isset($movie['poster_path']) ? "https://image.tmdb.org/t/p/w500{$movie['poster_path']}" : 'Not available',
                    );
                }
            }
            // Recomendations will be displayed and if there is none an error message displayed 
            return $recommendations;
        } else {
            return array('error' => 'Error retrieving movie credits.');
        }
    } else {
        return array('error' => 'Director not found.');
    }
}
// This function takes the parameter $username 
function getMoviesByMovieAndGenre($username) {
    // Key connects to the movie database
    global $apiKey;
    //  Connects to database using our credentials. 
    $conn = new mysqli('10.244.1.2', 'BackEndAdmin', 'Qg5OKQ!?$Q', 'SceneSync');
    // An empty arry is returned if there is a connection error. 
    if ($conn->connect_error) {
        return [];
    }
    // gets the account id with the username
    $accountId = getAccountIdByUsername($username);
    // Favorite genre and movie is taken with the account id
    $sql = "SELECT FavoriteGenres, FavoriteMovie FROM Profiles WHERE AccountID = '$accountId'";
    $result = $conn->query($sql);
    // Checks if anything was found with the profile 
    if ($result->num_rows > 0) {
        $profileData = $result->fetch_assoc();
        $favoriteGenre = $profileData['FavoriteGenres'];
        $favoriteMovie = $profileData['FavoriteMovie'];
        // Finding the movies by genre with the api request 
        $tmdbApiUrl = 'https://api.themoviedb.org/3/';
        $tmdbDiscoverEndpoint = 'discover/movie';
        $apiEndpoint = $tmdbApiUrl . $tmdbDiscoverEndpoint . '?api_key=' . $apiKey . '&with_genres=' . urlencode($favoriteGenre);
        $apiResponse = file_get_contents($apiEndpoint);
        // If the api request has an error it will return false
        if ($apiResponse === false) {
            return [];
        }
        // Api response is decoded and the title, overview, and release date is returned 
        $moviesData = json_decode($apiResponse, true);
        $recommendedMovies = array_map(function ($movie) {
            return [
                'title' => $movie['title'],
                'overview' => $movie['overview'],
                'release_date' => $movie['release_date'],
            ];
        // There could be ten results that are recommended 
        }, $moviesData['results']);
        return array_slice($recommendedMovies, 0, 10);
    // If nothing is found an empty array is displayed 
    } else {
        return [];
    }
}
// This function takes the parameter $username 
function getAccountIdByUsername($username) {
    // This will connect to our databse using our credentials. If there is an error, a message is displayed 
    $conn = new mysqli('10.244.1.2', 'BackEndAdmin', 'Qg5OKQ!?$Q', 'SceneSync');
    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    }
    $username = $conn->real_escape_string($username);
    // Account ID is associated with the username 
    $sql = "SELECT AccountID FROM Accounts WHERE Username = '$username'";
    $result = $conn->query($sql);
    // If the user name was found it gets the account ID 
    if ($result->num_rows > 0) {
        $row = $result->fetch_assoc();
        $accountId = $row['AccountID'];
        $conn->close();
        return $accountId;
    // If the username was not found the database connection is closed and returns false
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
// This functions take the parameter $username 
function getRecentWatchedRecommendations($username) {
    // The accountID is retreived with the $username 
    $accountId = getAccountIdByUsername($username);
    // If no one is found it will say no username found 
    if (!$accountId) {
        return array('error' => 'Username not found.');
    }
    // Recommendations are found based on most recent watched 
    $mostRecentWatched = getMostRecentWatched($username);
    if (!$mostRecentWatched) {
        return getPopularRecommendations($apiKey);
    }
    // Ten or less recommended movie titles are returned 
    $recentTitle = $mostRecentWatched['MovieTitle'];
    $recentPosterPath = $mostRecentWatched['PosterURL'];
    $recommendations = searchMoviesByTitle($recentTitle);
    $recommendations += getPopularRecommendations($apiKey, 10 - count($recommendations));
    return $recommendations;
}
//isfar
// This function takes the parameter $title
function searchMoviesByTitle($title) {
    // The key used for the movie database 
    global $apiKey;
    // The api url is made with its parameters and what to search for 
    $url = "https://api.themoviedb.org/3/search/movie";
    $params = [
        'api_key' => $apiKey,
        'query' => urlencode($title),
    ];
    // Curl is set
    $url .= '?' . http_build_query($params);
    $ch = curl_init($url);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    $response = curl_exec($ch);
    curl_close($ch);
    // A result is put into $data
    $data = json_decode($response, true);
    // The if checks if there are results in $data. 
    if (isset($data['results'])) {
        $movies = [];
        // The title, poster, and release year are returned 
        foreach ($data['results'] as $result) {
            $movies[] = [
                'title' => $result['title'],
                'poster_path' => $result['poster_path'],
                'release_year' => $result['release_date'],
            ];
        }
        return $movies;
    // Otherwise it will print nothing if no results were found
    } else {
        return [];
    }
}

//isfar
// This function takes the parameter $username. 
function getMostRecentWatched($username) {
    global $apiKey;
    // Gets the account ID associated with the username
    $accountId = getAccountIdByUsername($username);
    // If the username is not found an error message is displayed 
    if (!$accountId) {
        return array('error' => 'Username not found.');
    }
    // Connects to our database using our credentials. 
    $conn = new mysqli('10.244.1.2', 'BackEndAdmin', 'Qg5OKQ!?$Q', 'SceneSync');
    // An error message is displayed if the connection gets displayed. 
    if ($conn->connect_error) {
        die("Connection failed: " . $conn->connect_error);
    }
    // Gets Movie, PosterURL, and year are taken from watched list associated with the particular account ID
    $sql = "SELECT Movie, PosterURL, Year
            FROM WatchedList
            WHERE AccountID = ?
            LIMIT 1";
    // If something is found it will print out the relevant details otherwise most recent watched is set to null. 
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
    // Connection is closed 
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

//isfar 
// Takes two parameters mediaID, and media type. It calls the two functions getMovieByDetails and getTVByDetails depending what the media type is. 
function getMediaDetails($mediaId, $mediaType) {
    // Checks to see if its a movie and if it is it will return the details 
    if ($mediaType === 'movie') {
        return getMoviesByDetails($mediaId);
    // If it's a TV show the the details are returned 
    } elseif ($mediaType === 'tv') {
        return getTVByDetails($mediaId);
    // if it's not a movie or tv show an error message is displayed 
    } else {
        return ['error' => 'Invalid media type.'];
    }
}





?>