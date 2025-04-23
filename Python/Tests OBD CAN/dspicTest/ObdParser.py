ObdPids = [
    {'pid': '010C', 'description': 'Engine RPM', 'units': 'rpm', 'formula': lambda d: (int(d[0], 16) * 256 + int(d[1], 16)) / 4},
    {'pid': '010D', 'description': 'Vehicle speed', 'units': 'km/h', 'formula': lambda d: int(d[0], 16)},
    {'pid': '0105', 'description': 'Coolant temp', 'units': 'C', 'formula': lambda d: int(d[0], 16) - 40},
    {'pid': '0111', 'description': 'Throttle', 'units': '%', 'formula': lambda d: int(d[0], 16) * 100 / 255},
]

class ObdParser:
    def __init__(self, pids):
        self.pids = pids

    def parseResponse(self, response):
        response_cleaned = response.replace(' ', '').replace('\r', '')

        if response_cleaned.startswith('7E8'):
            pid = response_cleaned[6:10]
            data = response_cleaned[10:]
        else:
            return "Invalid CAN Frame"

        datarr = [data[i:i+2] for i in range(0, len(data), 2)]

        for item in self.pids:
            if item['pid'] == pid:
                return item['formula'](datarr) if 'formula' in item else data

        return "PID not found"

if __name__ == "__main__":
    parser = ObdParser(ObdPids)
    print(parser.parseResponse('7E8 04 41 0C 1A F8 \r'))
