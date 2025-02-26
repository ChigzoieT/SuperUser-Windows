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

// Get the userkey from the URL parameter
$userkey = isset($_GET['userkey']) ? $_GET['userkey'] : '';

// Check if userkey is provided
if (empty($userkey)) {
    header('Content-Type: application/json');
    echo json_encode(["error" => "Missing required parameter: userkey"]);
    exit;
}

// Prepare the SQL statement to get the json_value using userkey
$query = "SELECT json_value FROM json_table WHERE json_key = ?";
$stmt = $conn->prepare($query);
if (!$stmt) {
    header('Content-Type: application/json');
    die(json_encode(["error" => "Prepare failed: " . $conn->error]));
}

$stmt->bind_param('s', $userkey);
$stmt->execute();
$result = $stmt->get_result();

// Check if a matching record is found
if ($result->num_rows > 0) {
    $row = $result->fetch_assoc();
    
    // Decode the json_value to get the filename and file
    $data = json_decode($row['json_value'], true);
    
    // Check if filename and file exist in the JSON data
    if (isset($data['filename']) && isset($data['file'])) {
        header('Content-Type: application/json');
        echo json_encode([
            "filename" => $data['filename'],
            "file"     => $data['file']
        ]);
    } else {
        header('Content-Type: application/json');
        echo json_encode(["error" => "No file data found for this userkey."]);
    }
} else {
    // No record found for the given userkey
    header('Content-Type: application/json');
    echo json_encode(["error" => "Invalid userkey or no data found."]);
}

$stmt->close();
$conn->close();
?>
