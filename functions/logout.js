// functions/logout.js

function logout() {
    // Clear session variables or perform logout-related tasks here
    // For example:
    // Remove all session variables
    sessionStorage.clear();
    // Redirect to index.html after logout
    window.location.href = 'index.html';
  }