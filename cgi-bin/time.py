#!/usr/bin/python3

import datetime

# Print HTTP headers
print("Content-Type: text/html;charset=utf-8\r\n\r\n")

# Get current time
current_time = datetime.datetime.strftime(datetime.datetime.now(), "%H:%M:%S")

# HTML content with embedded CSS
html_content = f"""
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
            display: flex;
            flex-direction: column;
            justify-content: space-between;
            font-family: Arial, sans-serif;
            background-color: #1C300DCC;
        }}
        .banner {{
            display: flex;
            width: 100%;
            height: 100vh;
            background: url(../images/cucumbers_invitation.jpg) center no-repeat;
            background-size: cover;
            background-position-y: 100%;
            flex-grow: 1; /* Make banner take all available space */
            justify-content: center;
            align-items: center;
            color: white;
            font-size: 3em;
            text-shadow: 2px 2px 4px rgba(0, 0, 0, 0.5);
        }}
        footer {{
            padding: 5px 20px 5px 20px;
            color: white;
            flex-shrink: 0; /* Ensure footer stays at the bottom */
            text-align: center;
            background-color: #1C300DCC;
        }}
        a.nav {{
            display: inline-flex;
            justify-content: space-between;
            align-items: center;
            padding: 10px 20px;
            background-color: #1C300DCC;
            color: #fff;
            text-decoration: none;
            font-size: 1.2em;
            margin: 5px;
        }}
        .timeCur {{
            font-size: 2em;
        }}
    </style>
    <title>Cucumbers Army - Current Time</title>
</head>
<body>

<div class="banner">
    <div class="timeCur">Current Time: {current_time}</div>
</div>

<footer>
    <a class="nav" href="/">Home</a>
</footer>

</body>
</html>
"""

# Print the HTML content
print(html_content)
