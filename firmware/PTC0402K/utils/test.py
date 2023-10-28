#!/bin/env python3
import pyvisa
import time
import json
import csv
from datetime import datetime

rm = pyvisa.ResourceManager("@py")
inst = rm.open_resource('ASRL/dev/ttyACM0::INSTR')

def query(queryString):
    result = inst.query(queryString)
    print(queryString, "\n", result)
    return result

def write(string):
    inst.write(string)
    print(string, "\n")

# basic query
query("*IDN?")
## infered output channel and write
#write("OUTP 1")
## explict output channel and write
#write("OUTP1 1")
## full form channel names, case insensitivity and compound statement
#write("OUTPUT0 0; output1 0")
## instrument query
temps = []
try:
    while (True):
        entry = {}
        entry["time"] = "{:%Y-%m-%d %H:%M:%S}".format(datetime.now())
        for channel in range(0,4):
            s = int(inst.query(f"MEAS:Stat{channel}?"))
            entry[f"raw ({channel})"] = hex(int(inst.query(f"MEAS:RAW{channel}?")[2:],16))
            entry[f"cold ({channel})"] = float(inst.query(f"MEAS:Cold{channel}?"))
            entry[f"hot ({channel})"] = float(inst.query(f"MEAS:Temp{channel}?")) if not s else "-"
        temps.append(entry)
        print(f"0: cold {entry['cold (0)']}\thot {entry['hot (0)']}")
        print(f"1: cold {entry['cold (1)']}\thot {entry['hot (1)']}")
        print(f"2: cold {entry['cold (2)']}\thot {entry['hot (2)']}")
        print(f"3: cold {entry['cold (3)']}\thot {entry['hot (3)']}")

        time.sleep(1)
except KeyboardInterrupt:
    pass

time_str = "{:%Y-%m-%d %H:%M:%S}".format(datetime.now())
filename = f"ptc0402k [{time_str}].csv"

with open(filename, 'w', newline='') as fout:
    #json.dump(temps, fout);
    w = csv.DictWriter(fout, temps[0].keys())
    w.writeheader()
    w.writerows(temps)

print(f"Data saved to <{filename}>")
