#!/usr/bin/python3
# This script is intended to be called from text_to_image.sh
# requires python 3.7.1 or higher
# install openai module with : python -m pip install openai
# on debian:  apt-get install python3-pip ; python3.9 -m pip install openai
# based on example here: https://realpython.com/generate-images-with-dalle-openai-api/
# You need to export OPENAI_API_KEY with your api key

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
    size="512x512",
    response_format="b64_json",
)

#file_name = "created_image.json"
#print(f"{file_name=}")

#with open(file_name, mode="w", encoding="utf-8") as file:
#    json.dump(response, file)


#with open(file_name, mode="r", encoding="utf-8") as file:
#    response = json.load(file)

for index, image_dict in enumerate(response["data"]):
    image_data = b64decode(image_dict["b64_json"])
    image_file = "output_aventure_location_image.png"
    with open(image_file, mode="wb") as png:
        png.write(image_data)
