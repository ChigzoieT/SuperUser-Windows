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
    header('Content-Type: application/json');
    die(json_encode(["error" => "Connection failed: " . $conn->connect_error]));
}

// Get the raw JSON input
$jsonInput = file_get_contents('php://input');
$data = json_decode($jsonInput, true);

// Check if required parameters are provided in the JSON payload
if (isset($data['userkey']) && isset($data['text'])) {
    // Use the 'userkey' from the JSON payload as the json_key in the database
    $jsonKey = $data['userkey'];
    $text    = $data['text'];
    $filename = isset($data['filename']) ? $data['filename'] : null;
    
    // Check if 'file' is provided and assign it as a string
    $fileContent = isset($data['file']) ? $data['file'] : null;

    // Create a JSON object with the provided text, file data, and filename
    $jsonValue = json_encode([
        "text" => $text,
        "file" => $fileContent,
        "filename" => $filename
    ]);
    
    // Prepare the SQL statement to insert json_value into json_table
    $query = "INSERT INTO json_table (json_key, json_value) VALUES (?, ?)";
    $stmt = $conn->prepare($query);
    if (!$stmt) {
        header('Content-Type: application/json');
        die(json_encode(["error" => "Prepare failed: " . $conn->error]));
    }
    
    $stmt->bind_param('ss', $jsonKey, $jsonValue);
    
    // Set the content type to JSON
    header('Content-Type: application/json');
    
    if ($stmt->execute()) {
        echo json_encode(["status" => "success", "message" => "Data inserted successfully."]);
    } else {
        echo json_encode(["error" => "Failed to insert data: " . $stmt->error]);
    }
    
    $stmt->close();
} else {
    // Required parameters are missing
    header('Content-Type: application/json');
    echo json_encode(["error" => "Missing required parameters."]);
}

$conn->close();
?>
