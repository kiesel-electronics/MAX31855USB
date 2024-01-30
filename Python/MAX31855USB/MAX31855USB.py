import pyvisa


def list_resources():
    rm = pyvisa.ResourceManager()
    return rm.list_resources()


class MAX31855USB:
    def __init__(self, resource_name: str):
        self.rm = pyvisa.ResourceManager()
        self.instrument = self.rm.open_resource(resource_name)
        self.instrument.read_termination = '\n'
        self.instrument.write_termination = '\n'

    def id(self):
        return self.instrument.query('*IDN?')

    def get_temperature(self):
        return self.instrument.query(':TEMPerature?').rstrip("\r").split(",")

    def get_internal_temperature(self):
        return self.instrument.query(':TEMPerature:INTern?').rstrip("\r").split(",")
