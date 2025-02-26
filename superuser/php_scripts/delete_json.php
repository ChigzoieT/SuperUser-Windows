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

// Check if 'userkey' is provided in the JSON payload
if (isset($data['userkey'])) {
    $userkey = $data['userkey'];

    // Prepare the SQL statement to delete entries with the matching json_key
    $query = "DELETE FROM json_table WHERE json_key = ?";
    $stmt = $conn->prepare($query);
    if (!$stmt) {
        header('Content-Type: application/json');
        die(json_encode(["error" => "Prepare failed: " . $conn->error]));
    }

    $stmt->bind_param('s', $userkey);

    // Set the content type to JSON
    header('Content-Type: application/json');

    if ($stmt->execute()) {
        // Check if any rows were affected
        if ($stmt->affected_rows > 0) {
            echo json_encode(["status" => "success", "message" => "Data deleted successfully."]);
        } else {
            echo json_encode(["status" => "success", "message" => "No data found for the provided userkey."]);
        }
    } else {
        echo json_encode(["error" => "Failed to delete data: " . $stmt->error]);
    }

    $stmt->close();
} else {
    // 'userkey' is missing
    header('Content-Type: application/json');
    echo json_encode(["error" => "Missing required parameter: userkey."]);
}

$conn->close();
?>