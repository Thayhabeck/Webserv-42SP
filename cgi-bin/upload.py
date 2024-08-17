#!/usr/bin/python3

import cgi, os

form = cgi.FieldStorage()

# Get filename here
fileitem = form['filename']

# Define the path to the directory where files will be uploaded
upload_dir = os.path.join(os.getcwd(), 'www', 'cats_army', 'docs')

# Create the directory if it doesn't exist
if not os.path.exists(upload_dir):
    os.makedirs(upload_dir)

# Test if the file was uploaded
if fileitem.filename:
    filepath = os.path.join(upload_dir, os.path.basename(fileitem.filename))
    with open(filepath, 'wb') as f:
        f.write(fileitem.file.read())
    message = 'The file was uploaded to docs/'
else:
    message = 'Uploading Failed'

# Generate the HTML response
print("Content-Type: text/html;charset=utf-8")
print()

html_response = f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <style>
        html {{
            height: 100%;
        }}
        body {{
            height: 100%;
            margin: 0;
            font-family: 'Montserrat', sans-serif;   
            font-size: 15px;
            line-height: 1.6;
        }}
        .banner {{
            display:flex;
            flex-direction: column;
            justify-content: space-between;
            width: 100%;
            height: 100vh;
            background-size: cover;
            background-position-y: 0%;
            padding-top: 150px;
        }}
        .header {{
            display: block;
            width: 100%;
            padding-top: 10px;
            position: absolute;
            top: 0;
            left: 0;
            right: 0;
            z-index: 1000;
            background-color: #1C300DCC;
        }}
        .header_container {{
            display: flex;
            justify-content: space-between;
            align-items: center;
            font-weight: 700;
            font-size: 36px;
            text-transform: uppercase;
        }}
        /*Navigation button*/
        .nav {{
            font-size: 24px;
        }}
        .nav_link {{
            display: inline-block;
            vertical-align: top;
            margin: 0 24px;
            position: relative;
            color: grey;
            text-decoration: none;
            transition: color .2s linear;
        }}
        .nav_link:hover {{
            color: white;
        }}
        footer {{
            width: 100%;
            padding-left: 20px;
            color: black;
        }}
</style>
    <title>Upload Result - Cats Army</title>
</head>
<body>

<header class="header">
    <div class="header_container">
        <a class="nav_link" href="/">Cats Army</a>
        <nav class="nav">
            <a class="nav_link" href="/">About US</a>
            <a class="nav_link" href="/">JOIN US</a>
            <a class="nav_link" href="/docs">DOCS</a>
            <a class="nav_link" href="/cgi-bin">CGI</a>
            <a class="nav_link" href="/">Confidential</a>
        </nav>
    </div>
</header>

<div class="banner">
    <div class="container">
        <h1 style='color:#1C300D; text-align:center;'>{message}</h1>
    </div>
</div>

<footer>
    <p><span class="highlight">&#169; 2024 by WebServ thabeck- matcardo</p>
</footer>

</body>
</html>
"""

print(html_response)
