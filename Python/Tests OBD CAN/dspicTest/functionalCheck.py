import json
import time
from .dspicCommands import isFloat

class dspicFunctional:
    def __init__(self, connectionDspic):
        self.__connectionDspic = connectionDspic
        print("Functional Test Class initialized!")

    def ReadVoltage(self):
        dspic = self.__connectionDspic
        FunctionTestName = "Voltage read"
        LowLimit = 11.0
        HighLimit = 13.0

        dspic.SimpleCommandForTest(b'ATZ', False)
        time.sleep(1.0)
        dspic.SimpleCommandForTest(b'ATE0', False)
        resultV = dspic.SimpleCommandForTest(b'STVR', False)
        resultV = resultV.strip().translate(str.maketrans('', '', '\r>'))

        if isFloat(resultV):
            valueV = float(resultV)
            print(f"Voltage: {valueV}V")
            if LowLimit < valueV < HighLimit:
                return True
            else:
                print("Voltage out of range!")
                return False
        else:
            print(f"Invalid voltage response: {resultV}")
            return False
