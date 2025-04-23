import serial
import time
from abc import ABC, abstractmethod

ProtocolHashTableFreematics = {
    "HS_500_11_FIX": 6,
    "HS_500_29_FIX": 7,
    "HS_250_11_FIX": 8,
    "HS_250_29_FIX": 9,
    "MS_125_11_FIX": 10,
    "MS_125_29_FIX": 11
}

class EcuEmulator(ABC):
    @abstractmethod
    def __init__(self, port, baudrate): pass

    @abstractmethod
    def queryTestCmd(self, strCmd, WaitTime): pass

    @abstractmethod
    def set_can_protocol(self, string_proto): pass

    @abstractmethod
    def set_pid(self, pidstr, valueHexStr): pass

    @abstractmethod
    def reset(self): pass

    @abstractmethod
    def echo(self, eco): pass

    @abstractmethod
    def default(self): pass

class Freematics(EcuEmulator):
    def __init__(self, port, baudrate=9600):
        self._uart = serial.Serial(port, baudrate, timeout=1)

    def queryTestCmd(self, strCmd, WaitTime=0.3):
        self._uart.reset_output_buffer()
        self._uart.write((strCmd + "\r").encode())
        time.sleep(WaitTime)
        return self._uart.read_all().decode('utf-8')

    def set_can_protocol(self, string_proto):
        proto_id = ProtocolHashTableFreematics.get(string_proto)
        if proto_id is None:
            raise Exception(f"Protocol {string_proto} not supported")
        return self.queryTestCmd(f"ATSP{proto_id}")

    def set_pid(self, pidstr, valueHexStr):
        if isinstance(pidstr, bytes):
            pidstr = pidstr.decode('ANSI')
        return self.queryTestCmd(f"ATPAR{pidstr[-2:]},{valueHexStr}")

    def reset(self):
        return self.queryTestCmd("ATZ", 3.0)

    def echo(self, eco):
        return self.queryTestCmd(f"ATE{eco}")

    def default(self):
        return self.queryTestCmd("ATD")

class EmulatorFactory:
    @staticmethod
    def create(emu_type, port, baudrate):
        if emu_type.lower() == "freematics":
            return Freematics(port, baudrate)
        else:
            raise ValueError(f"Unknown emulator type: {emu_type}")
