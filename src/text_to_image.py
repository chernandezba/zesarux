#!/usr/bin/python3.9
# create.py

import json
import os
from pathlib import Path
from base64 import b64decode
import sys

import openai

#PROMPT = "                    EmbarcaderoViejas canoas se pudren al sol y un       decrepito muelle aun resiste las olas.     1/4"
PROMPT=sys.argv[1]

openai.api_key = os.getenv("OPENAI_API_KEY")

response = openai.Image.create(
    prompt=PROMPT,
    n=1,
    size="256x256",
    response_format="b64_json",
)

file_name = "created_image.json"
print(f"{file_name=}")

with open(file_name, mode="w", encoding="utf-8") as file:
    json.dump(response, file)


with open(file_name, mode="r", encoding="utf-8") as file:
    response = json.load(file)

for index, image_dict in enumerate(response["data"]):
    image_data = b64decode(image_dict["b64_json"])
    image_file = "created_image.png"
    with open(image_file, mode="wb") as png:
        png.write(image_data)
