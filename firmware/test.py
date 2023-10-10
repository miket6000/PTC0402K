import pyvisa
import time
from datetime import datetime

rm = pyvisa.ResourceManager("@py")
inst = rm.open_resource('ASRL/dev/ttyACM0::INSTR')

def query(queryString):
    result = inst.query(queryString)
    print(queryString, "\n", result)
    return result

def write(string):
    print(string, "\n", inst.write(string))

# basic query
query("*IDN?")
# infered output channel and write
write("OUTP 1")
# explict output channel and write
write("OUTP1 1")
# full form channel names, case insensitivity and compound statement
write("OUTPUT0 0; output1 0")
# instrument query
temps = {}
while (1):
    #temps["time"] = "{:%Y-%m-%d %H:%M:%S}".format(datetime.now())
    for channel in range(0,4):
        c = float(inst.query(f"MEAS:Cold{channel}?"))
        s = int(inst.query(f"MEAS:Stat{channel}?"))
        if (s):
            h = None
        else:
            h = float(inst.query(f"MEAS:Temp{channel}?"))
            r = int(inst.query(f"MEAS:RAW{channel}?")[2:],16)
            print(f"raw:{hex(r)}\tcold:{c}\thot:{h}")
        #temps[channel] = [c, h]
    #print (temps)
    time.sleep(1)

#query("MEAS:STAT0?")
#query("MEAS:COLD0?")

#query("MEAS:RAW1?")
#query("MEAS:STAT1?")
#query("MEAS:COLD1?")
