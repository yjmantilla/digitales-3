import serial
import serial.tools.list_ports
from threading import Thread,Event
from PyQt5.QtCore import QObject,pyqtSignal,pyqtSlot

class OurSerial(QObject):
    signal_data_available = pyqtSignal(str) 
    def __init__(self):
        super().__init__()
        self.serialPort = serial.Serial()
        self.serialPort.timeout = 1 # timeout in seconds

        self.scope = f'{type(self).__name__}'


        self.list_bauds = [1200*n for n in range(1,10)]
        self.dict_bauds = {str(x):x for x in self.list_bauds}
        self.list_ports =[]

        self.thread = None
        self.alive = Event()

    def log(self,msg):
        print(self.scope+'::'+msg)

    def refresh_ports(self):
        self.list_ports = [port.device for port in serial.tools.list_ports.comports()]
        self.log('ports\n'+str(self.list_ports))

    def connect_serial(self):
        try:
            self.serialPort.open()
        except Exception as e:
            self.log(str(e))
        if(self.serialPort.is_open):
            self.start_thread()

    def disconnect_serial(self):
        self.log("disconnecting...")
        self.stop_thread()
        self.serialPort.close()

    def read_serial(self):
        self.log("reading...")
        while (self.alive.isSet() and self.serialPort.is_open):
            data = self.serialPort.readline().decode("utf-8").strip()
            if True: #len(data)>1:
                #self.log(data)
                self.signal_data_available.emit(data)
    def send_data(self,data):
        if(self.serialPort.is_open):
            message = str(data) + "\n"
            self.serialPort.write(message.encode())
        else:
            self.log("No port open to send data")

    def start_thread(self):
        self.thread = Thread(target=self.read_serial)
        self.thread.setDaemon(1)
        self.alive.set()
        self.thread.start()
    
    def stop_thread(self):
        if (self.thread is not None):
            self.alive.clear()
            self.thread.join()
            self.thread = None

    def closeEvent(self,e):
        self.stop_thread()
        self.serialPort.close()