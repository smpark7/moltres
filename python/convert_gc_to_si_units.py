import argparse
import sys
import json
import numpy as np


def convert(input, output):
    gc_to_convert = ['FISSXS', 'GTRANSFXS', 'NSF', 'RECIPVEL', 'REMXS']
    with open(input, 'r') as file:
        data = json.load(file)
    for material, temperatures in data.items():
        for temperature, gcs in temperatures.items():
            if temperature != 'temp':
                for gc, values in gcs.items():
                    value = np.array(data[material][temperature][gc])
                    if gc == 'DIFFCOEF':
                        data[material][temperature][gc] = list(value / 100)
                    elif gc in gc_to_convert:
                        data[material][temperature][gc] = list(value * 100)
    with open(output, 'w') as file:
        json.dump(data, file, indent=4)


if __name__ == '__main__':

    parser = argparse.ArgumentParser(
        description='Converts group constant data length unit from cm to m')
    parser.add_argument(
        "input_filename",
        type=str,
        help="*.json file produced using moltres_xs.py")
    parser.add_argument(
        "output_filename",
        type=str,
        help="*.json file converted to meter units")
    args = parser.parse_args()

    convert(sys.argv[1], sys.argv[2])

    print("Successfully converted group constant data length unit from cm to m"
          )
