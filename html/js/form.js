function handleErrors(response) {
    if (!response.ok) {
        throw Error(response.statusText);
    }
    return response;
}

function uploadData()
{
    const input = document.getElementById("file");
    const formData = new FormData();
    formData.append(input.files[0].name, input.files[0]);
    fetch('/form_upload', {
        method: 'POST',
        credentials: 'include', 
        body: formData,
    })
    .then(handleErrors)
    .then((response) => {
        if (response.ok) {
            window.location.href = "form_upload_ok.html";
            return;
        }
        return response.text();
    })
    .then((response) => {
        console.log(response);
        document.body.innerHTML = response;
        window.location.reload();
    })
    .catch(error => console.log(error))
}
