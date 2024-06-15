#!/usr/bin/env python

import base64
import json
import math
import os
import re
import sys
import tempfile

from PIL import Image, ImageColor

def get_color(colhex, mode="RGBA"):
    return ImageColor.getcolor(f"#{colhex}", mode)

def dump_data(filename, data, palette, areas):
    print(f"len(palette) = {len(palette)}")
    print(f"len(areas) = {len(areas)}")
    for pal, ar in zip(palette, areas):
        print(f"#{pal} is {ar["MapArea"]}")
    size = int(math.ceil(math.sqrt(len(data))))
    print(f"Data length is {len(data)}, drawing to {size}x{size}")
    img = Image.new("L", (size, size))
    img.putpalette([a for col in palette for a in get_color(col)], "RGBA")
    img.putdata(data)
    img.save(os.path.splitext(filename)[0]+".png")

def main(filename):
    jsondata = json.load(open(filename))
    for something in jsondata:
        match something:
            case {"mAreaData": b64data, "mColorPalette": palette, "mColorToArea": areas}:
                dump_data(filename, base64.b64decode(b64data), palette, areas)
                break
            case _:
                print(f"Found Whiskey Tango Foxtrot: {dir(something).join(", ")}")

if __name__ == "__main__":
    if len(sys.argv) > 1:
        print(f"Hello {sys.argv[0]}: {sys.argv[1]}")
        main(sys.argv[1])
    else:
        main("MapAreaDump_2024.03.05-11.57.55.json")