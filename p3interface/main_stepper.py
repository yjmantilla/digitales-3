"""Based on https://www.youtube.com/watch?v=ppz1e_ceeIw
    https://github.com/Hikki12/Simple-Serial-PyQt5
"""
from re import S
import sys
from PyQt5.QtWidgets import QMainWindow,QApplication
from stepper_gui import Ui_MainWindow
from stepper import StepperMotor
from our_serial import OurSerial


class TheApp(QMainWindow):
    def __init__(self):
        super().__init__() # Heredar atributeos de QMainWindow
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)
        self.serial = OurSerial()
        self.stepper = StepperMotor()

        self.scope = f'{type(self).__name__}'

        # Configuring static comboboxes
        self.ui.comboBox_baudrate.addItems(self.serial.dict_bauds.keys())
        self.ui.comboBox_baudrate.setCurrentText('9600')
        self.ui.comboBox_direction.addItems(self.stepper.dict_directions.keys())
        self.ui.lineEdit_degrees.setText('45')
        self.ui.lineEdit_pause.setText('1')
        self.refresh()

        # EVENTS
        self.ui.pushButton_connect.clicked.connect(self.connect)
        self.ui.pushButton_send.clicked.connect(self.send_data)
        self.ui.pushButton_refresh.clicked.connect(self.refresh)
        self.ui.pushButton_clearAll.clicked.connect(self.clear)
        self.serial.signal_data_available.connect(self.refresh_terminal)

    def refresh_terminal(self,data):
        self.ui.textBrowser_sequence.append(data)

    def log(self,msg):
        print(self.scope+'::'+msg)

    def connect(self):
        if(self.ui.pushButton_connect.isChecked()):
            self.log('Trying to connect')
            port = self.ui.comboBox_port.currentText()
            baud = self.ui.comboBox_baudrate.currentText()
            self.serial.serialPort.port = port
            self.serial.serialPort.baudrate = baud
            self.serial.connect_serial()

            if (self.serial.serialPort.is_open):
                self.log("Connect Succesful")
                self.ui.pushButton_connect.setText("DISCONNECT")
            else:
                self.log("Connect Unsuccesful")
                self.ui.pushButton_connect.setChecked(False)
                self.ui.pushButton_connect.setText("CONNECT")
        else: # Disconnect
            self.serial.disconnect_serial()
            self.ui.pushButton_connect.setText("CONNECT")


    def send_data(self):
        data = self.ui.textBrowser_sequence.text()
        self.serial.send_data(data)
    
    def read_data(self):
        self.log("reading...")

    def refresh(self):
        self.serial.refresh_ports()
        self.ui.comboBox_port.clear()
        self.ui.comboBox_port.addItems(self.serial.list_ports)

    def clear(self):
        self.ui.textBrowser_sequence.clear()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = TheApp()
    window.show()
    sys.exit(app.exec_())
