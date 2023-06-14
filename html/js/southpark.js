// Votre JavaScript ici
document.addEventListener('DOMContentLoaded', function()
{
    const characters = document.querySelectorAll('.characters');
    console.log(characters);
    console.log(characters.getAttribute('children'));
    const overlay = document.getElementById('overlay');
    const videoPlayer = document.getElementById('videoPlayer');

    characters.forEach(character =>
    {
        character.addEventListener('click', function()
        {
            const videoPath = character.getAttribute('data-video');
            console.log("video path :" + videoPath);
            videoPlayer.src = videoPath;
            overlay.style.display = 'flex';
            videoPlayer.play();
        });
    });

    overlay.addEventListener('click', function()
    {
        videoPlayer.pause();
        videoPlayer.src = '';
        overlay.style.display = 'none';
    });
});