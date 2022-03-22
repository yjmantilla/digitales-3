from re import S
import sys
from PyQt5.QtWidgets import QMainWindow,QApplication
from terminal_gui import Ui_MainWindow
from our_serial import OurSerial


class TheApp(QMainWindow):
    def __init__(self):
        super().__init__() # Heredar atributeos de QMainWindow
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)
        self.serial = OurSerial()

        self.scope = f'{type(self).__name__}'

        # Configuring static comboboxes
        self.ui.combo_baud.addItems(self.serial.dict_bauds.keys())
        self.ui.combo_baud.setCurrentText('9600')
        # EVENTS
        self.ui.btn_connect.clicked.connect(self.connect)
        self.ui.btn_send.clicked.connect(self.send_data)
        self.ui.btn_refresh.clicked.connect(self.refresh)
        self.ui.btn_clear.clicked.connect(self.clear)

    def log(self,msg):
        print(self.scope+'::'+msg)

    def connect(self):
        if(self.ui.btn_connect.isChecked()):
            self.log('Trying to connect')
            port = self.ui.combo_port.currentText()
            baud = self.ui.combo_baud.currentText()
            self.serial.serialPort.port = port
            self.serial.serialPort.baudrate = baud
            self.serial.connect_serial()

            if (self.serial.serialPort.is_open):
                self.log("Connect Succesful")
            else:
                self.log("Connect Unsuccesful")
                #self.ui.btn_connect.isChecked = False
        else: # Disconnect
            self.log('disconnecting...')


    def send_data(self):
        self.log('sending...')
    
    def read_data(self):
        self.log("reading...")

    def refresh(self):
        self.serial.refresh_ports()
        self.ui.combo_port.clear()
        self.ui.combo_port.addItems(self.serial.list_ports)

    def clear(self):
        self.log("cleaning...")


if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = TheApp()
    window.show()
    sys.exit(app.exec_())
