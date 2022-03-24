"""Based on https://www.youtube.com/watch?v=ppz1e_ceeIw
    https://github.com/Hikki12/Simple-Serial-PyQt5
"""
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
        self.refresh()

        # EVENTS
        self.ui.btn_connect.clicked.connect(self.connect)
        self.ui.btn_send.clicked.connect(self.send_data)
        self.ui.btn_refresh.clicked.connect(self.refresh)
        self.ui.btn_clear.clicked.connect(self.clear)
        self.serial.signal_data_available.connect(self.refresh_terminal)

    def refresh_terminal(self,data):
        self.ui.textBrowser_terminal.append(data)

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
                self.ui.btn_connect.setText("DISCONNECT")
            else:
                self.log("Connect Unsuccesful")
                self.ui.btn_connect.setChecked(False)
                self.ui.btn_connect.setText("CONNECT")
        else: # Disconnect
            self.serial.disconnect_serial()
            self.ui.btn_connect.setText("CONNECT")


    def send_data(self):
        data = self.ui.lineEdit_input.text()
        self.serial.send_data(data)
    
    def read_data(self):
        self.log("reading...")

    def refresh(self):
        self.serial.refresh_ports()
        self.ui.combo_port.clear()
        self.ui.combo_port.addItems(self.serial.list_ports)

    def clear(self):
        self.ui.textBrowser_terminal.clear()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = TheApp()
    window.show()
    sys.exit(app.exec_())
