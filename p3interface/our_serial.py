import serial
import serial.tools.list_ports

class OurSerial(object):
    def __init__(self):
        super().__init__()
        self.serialPort = serial.Serial()

        self.scope = f'{type(self).__name__}'


        self.list_bauds = [1200*n for n in range(1,10)]
        self.dict_bauds = {str(x):x for x in self.list_bauds}
        self.list_ports =[]

    def log(self,msg):
        print(self.scope+msg)

    def refresh_ports(self):
        self.list_ports = [port.device for port in serial.tools.list_ports.comports()]
        self.log('ports\n'+str(self.list_ports))

    def connect_serial(self):
        try:
            self.serialPort.open()
        except Exception as e:
            self.log(str(e))
    def disconnect_serial(self):
        self.log("disconnecting...")

    def read_serial(self):
        self.log("reading...")

    def send_data(self,data):
        self.log("sending...",data)
