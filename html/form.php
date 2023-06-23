<!DOCTYPE html>
<head>
	<meta charset="UTF-8">
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
	<header>
		<div>
			<a href="index.html">Home</a>
		</div>
	</header>
	<div class="container">
		<h1>Upload de fichier<h1>
		<h3>(With CGI version(PHP))</h3>

		<?php
		if (isset($_POST['submit'])) {
			$target_dir = "uploads/";
			if (!is_dir($target_dir)) {
				mkdir($target_dir, 0777, true);
			}
			$target_file = $target_dir . basename($_FILES["fileToUpload"]["name"]);

			if (move_uploaded_file($_FILES["fileToUpload"]["tmp_name"], $target_file)) {
				// Redirect to upload_ok.html after successful upload
				echo '<div class="message success">Le fichier "' . basename($_FILES["fileToUpload"]["name"]) . '" a été téléchargé avec succès.</div>';
				//header('Location: upload_ok.html');
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

        <!-- form without cgi -->
		<h1>Upload de fichier</h1> 
		<h3>(Without CGI version (JavaScript))</h3>

		<form id="js-upload-form" enctype="multipart/form-data">
			<div class="text-center">
			</div>
			<div>
				<input type="file" id="file" name="file">
			</div>
			<div class="d-flex justify-content-center">
				<button type="button" onclick="uploadData()">SEND</button>
			</div>
		</form>
	</div>

	<div class="container">
	<h1>Supprimer un fichier</h1>
    
    <form id="deleteForm">
        <label for="filename">Nom du fichier :</label>
        <input type="text" id="filename" name="filename">
        <button type="button" onclick="deleteFile()">Supprimer</button>
    </form>

    <script>
		function deleteFile()
		{
			const filenameInput = document.getElementById('filename');
			const filename = filenameInput.value;

			const request = new XMLHttpRequest();
			request.open('DELETE', filename);
			request.onload = function()
			{
				const deleteForm = document.getElementById('deleteForm');
				if (request.status === 200)
				{
					console.log('ok');
					deleteForm.innerHTML = '<div class="message success">Le fichier "' + filename + '" a été supprimé avec succès.</div>';
				}
				else
				{
					console.log('error');
					deleteForm.innerHTML = '<div class="message error">Échec de la suppression du fichier! Error ' + request.status + '</div>';
				}
			};
			request.onerror = function()
			{
				deleteForm.innerHTML = 'Erreur lors de la requête DELETE';
			};
			request.send();
		}
	</script>
	<a href="checkupload.py">View uploaded files</a>
	<class="download">
		<h1>Téléchargement de fichier</h1>
		<p>Entrez le nom du fichier que vous souhaitez télécharger :</p>
		<input type="text" id="downloadname" required>
		<button onclick="downloadFile()">Télécharger</button>
	</div>
	<script type="text/javascript" src="js/form.js"></script>
</body>
</html>