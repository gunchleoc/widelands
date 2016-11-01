#!/usr/bin/env python
# encoding: utf-8

# TODO(GunChleoc): This converted the spritemaps that we already have from the conf files.
# We still need to be able to create spritemaps for more nonpacked animations.

import codecs
import json
import os.path
import sys

def parse_conf(directory, unit_name):
    regions = []
    def add_current_regions(regions):
        result = ""
        for region in regions:
            split_string = region.split(":")
            if (len(split_string)) == 2:
                dimensions = split_string[0].split(" ")
                if (len(dimensions)) == 4:
                    result += "         {\n"
                    result += "            rectangle = {" + dimensions[0] + ", " + dimensions[1] + ", " + dimensions[2] + ", " + dimensions[3] + "},\n"
                    result += "            offsets = {"
                    for offset in split_string[1].split(";"):
                        offset_x_y = offset.split(" ")
                        result += " {" + offset_x_y[0] + ", " + offset_x_y[1] + "},"
                    result = result[0:-1]
                    result += " },\n"
                    result += "         },\n"
        if (result):
            result = "      regions = {\n" + result + "      },\n"
        return result

    output = ""
    current_section = ""
    needs_parsing = False
    with open(directory + "/conf", "r") as lines:
        for aline in lines:
            line = aline.rstrip('\n')
            if line.startswith("[") and line.endswith("]"):
                if current_section and needs_parsing:
                    output += "   },\n"
                    output += add_current_regions(regions)
                    regions = []

                current_section = line[1:-1]
                needs_parsing = False
                #print("Parsing " + current_section)
            #else:
            #    current_section = ""
            if current_section:
                if line == "packed=true":
                    needs_parsing = True
                    output += "   " + current_section + " = {\n"
                if needs_parsing:
                    split_string = line.split("=")
                    if (len(split_string)) == 2:
                        key = split_string[0]
                        value = split_string[1]
                        if key == "pics":
                            output += "      spritemap = \"" + value + "\",\n"
                        elif key == "base_offset":
                            base_offset = value.split(" ")
                            output += "      offset = {" + base_offset[0] + ", " + base_offset[1] + "},\n"
                        elif key == "dimensions":
                            dimensions = value.split(" ")
                            output += "      size = {" + dimensions[0] + ", " + dimensions[1] + "},\n"
                        elif key == "hotspot":
                            hotspot = value.split(" ")
                            output += "      hotspot = {" + hotspot[0] + ", " + hotspot[1] + "},\n"
                        elif key == "fps":
                            output += "      fps = " + value + ",\n"
                        elif key.split("_")[0] == "region":
                            regions.append(value)

    output += add_current_regions(regions)
    if (output):
        output = "-- Animation spritemap for " + unit_name + "\n\n" + "return {\n" + output
        output += "   },\n"
        output += "}\n"
        print("Writing animations for " + directory + "\n")
        dest_filepath = os.path.normpath(directory + "/animations.lua")
        dest_file = codecs.open(dest_filepath, encoding='utf-8', mode='w')
        dest_file.write(output)
    #else:
    #    print("No animations with spritemaps")

base_path = os.path.abspath(os.path.join(os.path.dirname(__file__),os.path.pardir))
tribes_dir = os.path.normpath(base_path + "/tribes")

if (not os.path.isdir(tribes_dir)):
	print("Error: Path " + tribes_dir + " not found.")
	sys.exit(1)

for tribe_dir in os.listdir(tribes_dir):
    tribe_dir = os.path.normpath(tribes_dir + "/" + tribe_dir)
    for unit_dir in os.listdir(tribe_dir):
        #print(unit_dir)
        if unit_dir != "pics" and unit_dir != "scripting":
            dir_to_parse = os.path.normpath(tribe_dir + "/" + unit_dir)
            if os.path.isdir(dir_to_parse):
                parse_conf(dir_to_parse, unit_dir)
