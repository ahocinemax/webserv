<?php
	if ( isset ( $_POST['submit'] ) )
	{
		$target_dir = "uploads/";
		$target_file = $target_dir . basename($_FILES["fileToUpload"]["name"]);

		if (move_uploaded_file($_FILES["fileToUpload"]["tmp_name"], $target_file))
		{
			// Redirect to upload_ok.html after successful upload
			header('Location: upload_ok.html');
			exit();
		}
		else
		{
			echo "Upload failed!";
		}
	}
?>