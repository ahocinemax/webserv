<?php
if(isset($_POST['submit'])){
    $fileToUpload = $_FILES['fileToUpload'];

    // Vérifier s'il y a eu une erreur lors de l'upload
    if($fileToUpload['error'] !== UPLOAD_ERR_OK){
        echo "Une erreur est survenue lors de l'upload du fichier.";
    } else {
        // Chemin de destination du fichier uploadé
        $uploadPath = "uploads/" . basename($fileToUpload['name']);

        // Déplacer le fichier uploadé vers le répertoire de destination
        if(move_uploaded_file($fileToUpload['tmp_name'], $uploadPath)){
            echo "Le fichier a été téléversé avec succès.";
        } else {
            echo "Une erreur est survenue lors de la sauvegarde du fichier.";
        }
    }
}
?>
