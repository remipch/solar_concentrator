#import tkinter as tk
#root = tk.Tk()

import requests

BASE_URL = 'https://fullhdwall.com/'

response = requests.get(f"{BASE_URL}/wp-content/uploads/2016/04/Yellow-Sun-Picture.jpeg")
print(type(response))

print(response.status_code)
print(response.headers['content-type'])
print(response.headers)
print(response.encoding)
print(type(response.text))

exit()

