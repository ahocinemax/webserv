#!/usr/bin/python3

import os
import cgi
import cgitb
cgitb.enable()

print("<h2>Uploaded files</h2>")
print("<ul>")

upload_dir = '/mnt/nfs/homes/mtsuji/Documents/level5/webserv/ahocine/html/uploads'
if not os.path.exists(upload_dir):
    print("<p>Directory 'uploads' does not exist.</p>")
    exit()

try:
    files = os.listdir(upload_dir)
except Exception as e:
    print(f"<p>Error occurred when listing directory: {str(e)}</p>")
    exit()

for file in files:
    print(f"<li>{file}</li>")

print("</ul>")
