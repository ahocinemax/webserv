<?php
//phpinfo();
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $target_dir = "/mnt/nfs/homes/mtsuji/Documents/level5/webserv/ahocine/html/uploads";
    $target_file = $target_dir . basename($_FILES["fileToUpload"]["name"]);
    
    if (move_uploaded_file($_FILES["fileToUpload"]["tmp_name"], $target_file)) {
        // Display message directly instead of redirecting
        echo "Upload OK!";
        exit();
    } else {
        echo "Upload failed! Reason: ";
        switch ($_FILES["fileToUpload"]["error"]) {
            case UPLOAD_ERR_OK:
                $message = "No errors.";
                break;
            case UPLOAD_ERR_INI_SIZE:
            case UPLOAD_ERR_FORM_SIZE:
                $message = "File too large.";
                break;
            case UPLOAD_ERR_PARTIAL:
                $message = "File upload was partial.";
                break;
            case UPLOAD_ERR_NO_FILE:
                $message = "No file was uploaded.";
                break;
            case UPLOAD_ERR_NO_TMP_DIR:
                $message = "Missing a temporary folder.";
                break;
            case UPLOAD_ERR_CANT_WRITE:
                $message = "Failed to write file to disk.";
                break;
            case UPLOAD_ERR_EXTENSION:
                $message = "File upload stopped by extension.";
                break;
            default:
                $message = "Unknown upload error.";
                break;
        }
        echo $message;
        exit();
    }
    
    print_r($_FILES);
}
?>

