<!DOCTYPE html>
<html>
<head>
	<title>Upload de fichier</title>
	<style>
		body {
			font-family: Arial, sans-serif;
			margin: 0;
			padding: 20px;
		}

		h1 {
			color: #333;
		}

		.container {
			max-width: 600px;
			margin: 0 auto;
		}

		.message {
			margin-top: 20px;
			padding: 10px;
			border-radius: 4px;
		}

		.success {
			background-color: #e0f2f1;
			color: #009688;
		}

		.error {
			background-color: #ffebee;
			color: #f44336;
		}
	</style>
</head>
<body>
	<div class="container">
		<h1>Upload de fichier</h1>

		<?php
		if (isset($_POST['submit'])) {
			$target_dir = "uploads/";
			$target_file = $target_dir . basename($_FILES["fileToUpload"]["name"]);

			if (move_uploaded_file($_FILES["fileToUpload"]["tmp_name"], $target_file)) {
				// Redirect to upload_ok.html after successful upload
				echo '<div class="message success">Le fichier "' . basename($_FILES["fileToUpload"]["name"]) . '" a été téléchargé avec succès.</div>';
				header('Location: upload_ok.html');
				exit();
			} else {
				echo '<div class="message error">Échec du téléchargement du fichier!</div>';
			}
		} else {
			echo '<div class="message error">Aucun fichier sélectionné!</div>';
		}
		?>

		<form action="" method="POST" enctype="multipart/form-data">
			<input type="file" name="fileToUpload" id="fileToUpload">
			<input type="submit" name="submit" value="Télécharger">
		</form>
	</div>
</body>
</html>
