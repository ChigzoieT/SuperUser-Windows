<?php
// Database connection details
$host     = 'localhost';
$username = 'your_username';
$password = 'your_password';
$database = 'your_database';

// Connect to the database
$conn = new mysqli($host, $username, $password, $database);

// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
}

// Check if 'userkey' parameter is provided (e.g., via GET)
if (isset($_GET['userkey'])) {
    $jsonKey = $_GET['userkey'];

    // Prepare the SQL statement to retrieve json_value from json_table
    $query = "SELECT json_value FROM json_table WHERE json_key = ?";
    $stmt = $conn->prepare($query);
    if (!$stmt) {
        die("Prepare failed: " . $conn->error);
    }

    $stmt->bind_param('s', $jsonKey);
    $stmt->execute();
    $result = $stmt->get_result();

    // Set the content type to JSON
    header('Content-Type: application/json');

    if ($result->num_rows > 0) {
        $row = $result->fetch_assoc();
        
        // Decode the json_value to access individual fields
        $jsonData = json_decode($row['json_value'], true);
        
        if (json_last_error() === JSON_ERROR_NONE) {
            // Handle null values and ensure filename is treated as a string
            $text = isset($jsonData['text']) ? $jsonData['text'] : null;
            $filename = isset($jsonData['filename']) ? (string) $jsonData['filename'] : null; // Ensure filename is a string or null
            
            echo json_encode([
                "text" => $text,
                "filename" => $filename  // Corrected to use $filename
            ]);
        } else {
            // Error in decoding JSON
            echo "failed";
        }
    } else {
        // No record found for the given userkey
        echo "failed";
    }

    $stmt->close();
} else {
    // Parameter 'userkey' is missing
    echo "failed";
}

// Close connection
$conn->close();
?>
