document.addEventListener('DOMContentLoaded', function() {
	const charactersTab = document.querySelectorAll('.character-card');
	const characters = Array.from(charactersTab);
	const overlay = document.getElementById('overlay');
	const videoPlayer = document.getElementById('videoPlayer');
	// il faut empecher la fenetre de s'afficher si la video n'existe pas
	
	characters.forEach(character => {
		character.addEventListener('click', function() {
			const videoPath = character.getAttribute('data-video');
			videoPlayer.src = videoPath;
			// si la réponse du serveur est 200, on affiche la video
			overlay.style.display = 'flex';
			videoPlayer.play();
		});
	});

	overlay.addEventListener('click', function() {
		closeVideo();
	});

	document.addEventListener('keydown', function(event) {
		if (event.key === 'Escape') {
			closeVideo();
		}
	});

	function closeVideo() {
		videoPlayer.pause();
		videoPlayer.src = '';
		overlay.style.display = 'none';
	}
});
