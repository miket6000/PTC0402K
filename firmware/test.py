import pyvisa
import time
import json
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
        for channel in range(0,4):
            s = int(inst.query(f"MEAS:Stat{channel}?"))
            if (s):
                h = None
            else:
                entry = {}
                entry["time"] = "{:%Y-%m-%d %H:%M:%S}".format(datetime.now())
                entry["channel"] = channel
                entry["raw"] = hex(int(inst.query(f"MEAS:RAW{channel}?")[2:],16))
                entry["cold"] = float(inst.query(f"MEAS:Cold{channel}?"))
                entry["hot"] = float(inst.query(f"MEAS:Temp{channel}?"))
                print(f"raw:{entry['raw']}\tcold:{entry['cold']}\thot:{entry['hot']}")
                temps.append(entry)
        time.sleep(1)
except KeyboardInterrupt:
    pass

time_str = "{:%Y-%m-%d %H:%M:%S}".format(datetime.now())
filename = f"ptc0402k [{time_str}].json"

with open(filename, 'w') as fout:
    json.dump(temps, fout);

print(f"Data saved to <{filename}>")
#query("MEAS:STAT0?")
#query("MEAS:COLD0?")

#query("MEAS:RAW1?")
#query("MEAS:STAT1?")
#query("MEAS:COLD1?")
