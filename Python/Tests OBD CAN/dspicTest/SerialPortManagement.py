import serial.tools.list_ports
import json

class SerialPortManagement:
    def __init__(self, output_file="serial_ports.json"):
        self.outputFile = output_file
        self.serialNumbers = {}
        self.loadSerialNumbers()

    def getUsbUartSerialNumbers(self):
        ports = list(serial.tools.list_ports.comports())
        for port in ports:
            if "USB" in port.hwid:
                try:
                    hwidParts = port.hwid.split("SER=", 1)
                    if len(hwidParts) == 2:
                        serialNumber = hwidParts[1].split(" ")[0]
                        self.serialNumbers[port.device] = {
                            "portName": port.device,
                            "userName": "",
                            "serial": serialNumber
                        }
                except Exception as e:
                    print(f"Error retrieving serial for {port.device}: {e}")

    def saveToJson(self):
        with open(self.outputFile, "w") as f:
            json.dump(self.serialNumbers, f, indent=4)

    def loadSerialNumbers(self):
        try:
            with open(self.outputFile, "r") as f:
                self.serialNumbers = json.load(f)
        except FileNotFoundError:
            self.serialNumbers = {}

    def getMappedPorts(self, userPorts=None):
        if userPorts is None:
            userPorts = [self.serialNumbers[sn]["userName"] for sn in self.serialNumbers]

        knownPorts = list(self.serialNumbers.values())
        mappedPorts = []

        for userPort in userPorts:
            for knownPort in knownPorts:
                if userPort == knownPort["userName"]:
                    serialNumber = knownPort["serial"]
                    for port in serial.tools.list_ports.comports():
                        if "USB" not in port.hwid:
                            continue
                        if serialNumber in port.hwid:
                            mappedPorts.append({
                                "userPort": userPort,
                                "serialNumber": serialNumber,
                                "comPort": port.device
                            })
                            break
                    else:
                        mappedPorts.append({
                            "userPort": userPort,
                            "serialNumber": serialNumber,
                            "comPort": knownPort["portName"]
                        })

        return mappedPorts

    def getComPortByUsername(self, username, mappedPortList=None):
        mappedPortList = mappedPortList or self.getMappedPorts()
        for mapping in mappedPortList:
            if mapping["userPort"] == username:
                return mapping["comPort"]
        return None
