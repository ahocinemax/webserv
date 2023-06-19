#!/usr/bin/python3
import cgi, os, os.path
import cgitb; cgitb.enable()

# File storage location
target_dir = "./html/tmp/"

# Enable receiving the user's request
form = cgi.FieldStorage()

# Get the file name that is supposed to be uploaded
file_item = form['fileToUpload']

def append_number_to_filename(filename, number):
    pos = filename.rfind(".")
    tmp = filename[0 : pos :]
    extension = filename[pos: :]
    tmp += "_" + str(number) + extension
    return (tmp)

def generate_filename(filename):
    index = 1
    tmp = os.path.join(target_dir, filename)
    while os.path.exists(tmp):
        tmp = os.path.join(target_dir, append_number_to_filename(filename, index))
        index += 1
    return (tmp)

# Check if the file has been uploaded
if file_item.filename:
    # Remove the main access path
    # from the file name to avoid problems
    fn = generate_filename(os.path.basename(file_item.filename))
    # Write and convert to binary
    open(fn, 'wb').write(file_item.file.read())
    message = 'The file "' + fn + '" was uploaded successfully'
else:
    message = 'No file uploaded :('

response = """Status: 201 Created\r\n
Content-type: text/html\r\n\r\n

<!DOCTYPE html>
<html>
    <head>
        <title>File Upload</title>
        <meta charset="UTF-8">
    </head>
    <body>
        <h1>
            <center>File Upload Result</center>
        </h1>
        <div>
            <center>{}</center>
        </div>
        <form action="form.php">
            <center><input type="submit" value="Upload a new file" /></center>
        </form>
        <form action="index.html">
            <center><input type="submit" value="Back to Home Page" /></center>
        </form>
    </body>
</html>
""".format(response)

sys.stdout.buffer.write(response.encode('utf-8'))