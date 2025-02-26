<?php
$host = 'localhost';
$username = 'u746292088_supuser';
$password = 'Checkmate123@@@';
$database = 'u746292088_supuser';

$conn = new mysqli($host, $username, $password, $database);
if ($conn->connect_error) {
    die("Error");
}

$jsonData = file_get_contents('php://input');
$data = json_decode($jsonData, true);

if (isset($data['userkey'], $data['password'], $data['state'])) {
    $userkey = $data['userkey'];
    $password = $data['password'];
    $state = $data['state'];

    if ($state == "new") {
        $stmt = $conn->prepare("SELECT COUNT(*) FROM users WHERE userkey = ?");
        $stmt->bind_param('s', $userkey);
        $stmt->execute();
        $stmt->bind_result($count);
        $stmt->fetch();
        $stmt->close();

        if ($count == 0) {
            $hashedPassword = password_hash($password, PASSWORD_BCRYPT);
            $stmt = $conn->prepare("INSERT INTO users (userkey, password, currentvalue) VALUES (?, ?, 0)");
            $stmt->bind_param('ss', $userkey, $hashedPassword);

            if ($stmt->execute()) {
                $stmt->close();
                $jsonKey = $userkey . "0";
                $jsonValue = json_encode(["text" => $data['cmptext'] ?? "", "file" => null, "filename" => null]);
                
                $stmt = $conn->prepare("INSERT INTO json_table (json_key, json_value) VALUES (?, ?)");
                $stmt->bind_param('ss', $jsonKey, $jsonValue);
                $stmt->execute();
                echo "success";
            }else{
                echo "error";
            }
            $stmt->close();
        }
    } elseif ($state == "old") {
        $stmt = $conn->prepare("SELECT password FROM users WHERE userkey = ?");
        $stmt->bind_param('s', $userkey);
        $stmt->execute();
        $stmt->bind_result($hashedPassword);
        $stmt->fetch();
        $stmt->close();

        if ($hashedPassword && password_verify($password, $hashedPassword)) {
            echo "success";
        }else{
            echo "error";
        }
    }
}
$conn->close();
