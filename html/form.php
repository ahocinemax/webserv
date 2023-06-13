<?php
//phpinfo();
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    // ディレクトリを変更
    $target_dir = "./tmp/";
    // basename()関数を使用してファイル名だけを取得し、それに一意のIDを付加します
    $target_file = $target_dir . uniqid() . basename($_FILES["fileToUpload"]["name"]);

    if (move_uploaded_file($_FILES["fileToUpload"]["tmp_name"], $target_file)) {
        // Redirect to upload_ok.html after successful upload
        header('Location: upload_ok.html');
        exit();
    } else {
        echo "Upload failed!";
    }
}
?>
