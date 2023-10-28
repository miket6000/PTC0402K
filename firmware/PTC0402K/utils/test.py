#!/bin/env python3
import pyvisa
import time
import json
import csv
from datetime import datetime

rm = pyvisa.ResourceManager("@py")
inst = rm.open_resource('ASRL/dev/ttyACM0::INSTR')
inst.write_termination = '\r\n'
inst.read_termination = '\r\n'

def query(queryString):
    result = inst.query(queryString)
    print(queryString, "\n", result)
    return result

def write(string):
    inst.write(string)
    print(string, "\n")

# Reset and clear state. Resetting sometimes causes garbage characters to be transmitted
# so we read any characters in the buffer and continue.
inst.query("*RST")
try:
    inst.timeout = 10
    inst.read_raw()
except:
    pass
inst.timeout = 1000
inst.write("*CLS")
query("*IDN?")

# setup a temp controller. You must set a limit, an output and an input BEFORE you enable.
inst.write("Alarm:Limit 20.0; output 1; input 2; enable 1")

# loop forever reading out all four channels. If we're interrupted by the keyboard record
# the data in a CSV file. This is not a robust way of doing this as nothing is written
# to file until the code is interrupted. We should periodically save data instead...
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

rm.close()
time_str = "{:%Y-%m-%d %H:%M:%S}".format(datetime.now())
filename = f"ptc0402k [{time_str}].csv"

with open(filename, 'w', newline='') as fout:
    w = csv.DictWriter(fout, temps[0].keys())
    w.writeheader()
    w.writerows(temps)

print(f"\nData saved to <{filename}>")
