import sys

err_codes = {
    # Failsafe Low Action Error Code Group¶
    0x4000000: "Fan over current (> 0.25 A)",
    0x100000: "Fuel cell's internal State is set 'stop' for > 15 s",
    0x20000: "Tank pressure < 15 barg",
    0x2000: "Stack 1 under temperature (< 5 degC)",
    0x1000: "Stack 2 under temperature (< 5 degC)",
    0x800: "Battery under voltage warning (21.6 V)",
    0x200: "Fan pulse aborted",
    0x100: "Stack under voltage (650 W < 17.4V, 800 W < 21.13 V)",
    0x80: "Stack under voltage and battery power below threshold (< -200 W)",
    0x10: "Battery charger fault",
    0x8: "Battery undertemperature (< -15 degC)",
    # Failsafe Critical Action Error Code Group
    0x80000000: "Stack 1 over temperature alert (>58 degC)",
    0x40000000: "Stack 2 over temperature alert (>58 degC)",
    0x20000000: "Battery under volt alert (<19 V)",
    0x10000000: "Battery over temperature alert (>65 degC)",
    0x8000000: "No fan current detected (<0.01 A)",
    0x2000000: "Stack 1 over temperature critical (>57 degC)",
    0x1000000: "Stack 2 over temperature critical (>57 degC)",
    0x800000: "Battery under volt warning (<19.6 V)",
    0x400000: "Battery over temperature warning (>60 degC)",
    0x200000: "Fuel cell's internal State == start for > 30 s",
    0x80000: "Tank pressure < 6 barg",
    0x40000: "Tank pressure < 5 barg",
    0x10000: "Fuel cell's internal safety flags not set true",
    0x8000: "Stack 1 denied start",
    0x4000: "Stack 2 denied start",
    0x400: "Battery under voltage (21.6 V) and master denied",
    0x40: "Over voltage and over current protection",
    0x20: "Invalid serial number"
}



def getErr(err):
    res = []
    codes = list(err_codes.keys())
    codes = sorted(codes, reverse=True)
    for code in codes:
        if(err-code >= 0):
            res.append(err_codes[code])
            err-=code
    if(err == 0):
        return res
    else:
        print("Error deconstructing error, rest:", hex(err), "res:", res)
        return None

if __name__ == '__main__':
    if(len(sys.argv) == 1):
        err = 0x00000010
        r = getErr(err)
        print(err)
        print(r)
    if(len(sys.argv) > 1):
        try:
            h = sys.argv[1]
            print(h)
            print(getErr(int(h, 16)))
            exit()
        except ValueError:
            file = sys.argv[1]
            with open(file, 'r') as f:
                i = -1
                while(True):
                    last_line = f.readlines()[i]
                    i-=1
                    for e in last_line.split(","):
                        if "0x" in e:
                            print(e)
                            print(getErr(int(e,16)))
                            exit() # yes this is scuffed, but it works