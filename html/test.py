#!/usr/bin/python3

import os

print("Content-type: text/html\r\n\r\n")
print("<font size=+1>Environment</font></br>")
print("<!DOCTYPE html>")
print("<html>")
print("<head>")
print("<title>CGI Environment Variables</title>")
print("</head>")
print("<body>")
print("<ul>")
for param in os.environ.keys():
   print("<li><b>%20s</b>: %s</li>" % (param, os.environ[param]))
print("</ul>")
print("</body>")
print("</html>")
