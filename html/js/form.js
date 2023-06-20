function handleErrors(response) {
    if (!response.ok) {
        throw Error(response.statusText);
    }
    return response;
}

function displayMessage(messageText, messageType) {
    var message = document.createElement('div');
    message.classList.add('message', messageType);
    message.textContent = messageText;

    var container = document.querySelector('.container');
    container.insertBefore(message, document.getElementById('js-upload-form'));
}

function uploadData()
{
    const input = document.getElementById("file");
    const formData = new FormData();
    
    if(input.files.length === 0) {
        displayMessage('Aucun fichier sélectionné!', 'error');
        return;
    }

    formData.append(input.files[0].name, input.files[0]);
    fetch('/form_upload', {
        method: 'POST',
        credentials: 'include', 
        body: formData,
    })
    .then(handleErrors)
    .then((response) => {
        if (response.ok) {
            displayMessage('Le fichier "' + input.files[0].name + '" a été téléchargé avec succès.', 'success');
            return;
        }
        return response.text();
    })
    .then((response) => {
        console.log(response);
        document.body.innerHTML = response;
        window.location.reload();
    })
    .catch(error => {
        console.log(error);
        displayMessage('Échec du téléchargement du fichier!', 'error');
    })
}