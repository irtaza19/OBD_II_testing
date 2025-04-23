import time
import serial
import json
import os.path
import re
from enum import Enum

JsonTestNameCommandTest = "Command Test"
JsonTestNameFunctionalTest = "Functional Test"
DspicBaudrate = 9600

ProtocolHashTableDspic = {
    "HS_500_11_FIX": 33,
    "HS_500_29_FIX": 34,
    "HS_250_11_FIX": 35,
    "HS_250_29_FIX": 36,
    "MS_125_11_FIX": 53,
    "MS_125_29_FIX": 54
}

def isFloat(strCheck):
    f = re.search("^[+-]?([0-9]+([.][0-9]*)?|[,.][0-9]+)$", strCheck)
    return not (f is None)

class TestScriptType(Enum):
    RecordResults = 1
    CompareResults = 2

class DspicTestEnvironment:
    def __init__(self, dspicTestPort, jsonFilePath, runType=TestScriptType.RecordResults):
        self.DspicTestPort = dspicTestPort
        self.JsonCompareFilePath = jsonFilePath
        self.__testRunType = runType
        self.ObdCommandosCollectorObj = {}

        if not os.path.isfile(self.JsonCompareFilePath):
            print(f"{self.JsonCompareFilePath} not found. Creating new object.")
        else:
            with open(self.JsonCompareFilePath) as f:
                self.ObdCommandosCollectorObj = json.load(f)

        self.ObdCommandosCollectorObj.setdefault(JsonTestNameCommandTest, {})
        self.ObdCommandosCollectorObj.setdefault(JsonTestNameFunctionalTest, {})

        self.__connection = serial.Serial(self.DspicTestPort, DspicBaudrate)
        if not self.__connection.is_open:
            raise Exception(f"Could not open COM port {self.DspicTestPort}")

    def __del__(self):
        if hasattr(self, '__connection') and self.__connection:
            self.__connection.close()

    def AddCommandToResult(self, cmd, result):
        NewCmdObj = self.ObdCommandosCollectorObj[JsonTestNameCommandTest].get(cmd, {"name": cmd})
        if self.__testRunType == TestScriptType.RecordResults:
            NewCmdObj["result_original"] = result
        else:
            NewCmdObj["result_compare"] = result
        self.ObdCommandosCollectorObj[JsonTestNameCommandTest][cmd] = NewCmdObj

    def SimpleCommandForTest(self, ObdCommand, includeInJsonCmdList=True, WaitTimeForAnswer=0.4):
        self.__connection.reset_output_buffer()
        if isinstance(ObdCommand, str):
            ObdCommand = ObdCommand.encode()
        self.__connection.write(ObdCommand + b"\r")
        time.sleep(WaitTimeForAnswer)

        try:
            rawStr = self.__connection.read_all().decode('ascii')
            strCmdUtf8 = ObdCommand.decode('utf-8')
        except Exception as e:
            print(f"Error while decoding answer: {e}")
            return ""

        print(f"Command {strCmdUtf8} returns:\n{repr(rawStr)}")

        if includeInJsonCmdList:
            self.AddCommandToResult(strCmdUtf8, rawStr)
            if self.__testRunType == TestScriptType.CompareResults:
                orig = self.ObdCommandosCollectorObj[JsonTestNameCommandTest][strCmdUtf8].get("result_original")
                result = "PASS" if rawStr == orig else "FAIL"
                self.ObdCommandosCollectorObj[JsonTestNameCommandTest][strCmdUtf8]["result"] = result
                if result == "FAIL":
                    print(f"Mismatch! Received: {rawStr}, Expected: {orig}")
                else:
                    print("PASS")

        return rawStr

    def Finish(self):
        with open(self.JsonCompareFilePath, "w") as outfile:
            json.dump(self.ObdCommandosCollectorObj, outfile, indent=4)
