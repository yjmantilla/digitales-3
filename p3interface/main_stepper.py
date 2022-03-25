"""Based on https://www.youtube.com/watch?v=ppz1e_ceeIw
    https://github.com/Hikki12/Simple-Serial-PyQt5
"""
from re import S
import sys
from PyQt5.QtWidgets import QMainWindow,QApplication,QMessageBox,QHeaderView
from PyQt5.QtCore import pyqtSlot,pyqtSignal
from PyQt5.QtGui import QStandardItemModel,QStandardItem
import copy
from stepper_gui import Ui_MainWindow
from stepper import StepperMotor
from our_serial import OurSerial


class TheApp(QMainWindow):
    report_signal = pyqtSignal(object)
    def __init__(self):
        super().__init__() # Heredar atributeos de QMainWindow
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)
        self.serial = OurSerial()
        self.stepper = StepperMotor()
        self.NITEMS = 10
        self.sequence_keys = ['direction','degrees','pause']
        # Index apunta a la siguiente posicion de la lista a llenarse
        self.sequence_init = {'index':0,'direction':['']*self.NITEMS,'degrees':['']*self.NITEMS,'pause':['']*self.NITEMS}
        self.sequence = copy.deepcopy(self.sequence_init)
        self.scope = f'{type(self).__name__}'

        # Table Model
        self.model = QStandardItemModel(4,3)
        #Establecer el encabezado
        self.model.setHorizontalHeaderLabels (['Direction','Degrees','Pause'])
        self.ui.tableView_sequence.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)
        self.ui.tableView_sequence.setModel(self.model)
        self.refresh_table()

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
        self.report_signal.connect(self.on_report_signal)
        self.ui.pushButton_addMove.clicked.connect(self.add_move)
        self.ui.pushButton_removeLast.clicked.connect(self.remove_last)
        self.ui.pushButton_clearAll.clicked.connect(self.clear_all)
    
    def clear_all(self):
        self.sequence = copy.deepcopy(self.sequence_init)
        self.refresh_table()
    def showMessage(self,tuple_args):
        """
        Method that emits a message through the ``self.report_signal``
        This allows to show a message box when a report is finished.
        tuple_args = (True|False|None,Message True,Message False)
        If None, no message box is shown.
        """
        self.report_signal.emit(tuple_args)


    @pyqtSlot(object)
    def on_report_signal(self,text):
        """
        An slot that shows a QMessageBox with the feedback of
        the report html job asked by the user.
        tuple_args = (True|False|None,Message True,Message False)
        If None, no message box is shown.
        """

        msg = QMessageBox(self)
        msg.setIcon(QMessageBox.Information)
        msg.setWindowTitle("Notice!")

        msg.setText(text)
        msg.show()
    def refresh_table(self):
        self.ui.tableView_sequence.horizontalHeader().setSectionResizeMode(QHeaderView.Stretch)

        for i in range(self.NITEMS):
            for j,key in enumerate(self.sequence_keys):
                print(i,j,self.sequence[key][i])
                self.model.setItem(i,j,QStandardItem(str(self.sequence[key][i])))
    def add_move(self):
        #TODO: Verify the table has less than N items
        valid_move = True
        if self.sequence['index']>self.NITEMS:
            self.showMessage('ERROR: Max limit of movements already reached.')
        try:
            degrees = float(self.ui.lineEdit_degrees.text())
            MAX_DEGREE = 1080
            MIN_DEGREE = 10
            if not (MIN_DEGREE <= degrees and degrees <= MAX_DEGREE):
                self.showMessage(f'Degrees have to be between {MIN_DEGREE} and {MAX_DEGREE}')
                valid_move = False
        except:
            self.showMessage('Couldnt convert degrees to float')
            valid_move = False

        direction = self.ui.comboBox_direction.currentText()

        try:
            pause = float(self.ui.lineEdit_pause.text())
            MAX_PAUSE = 5
            MIN_PAUSE = 1
            if not (MIN_PAUSE <= pause and pause <= MAX_PAUSE):
                self.showMessage(f'Pause have to be between {MIN_PAUSE} and {MAX_PAUSE}')
                valid_move = False
        except:
            self.showMessage('Couldnt convert pause to float')
            valid_move = False
        if valid_move == False:
            return
        currentMove = {'direction':direction,'degrees':degrees,'pause':pause}

        for key in currentMove.keys():
            self.sequence[key][self.sequence['index']]=currentMove[key]
        self.sequence['index'] +=1
        #TODO: Cycle back to start of the sequence after ten items?
        #TODO: Ignore empty movements, dont send to micro
        self.refresh_table()
        #print(dir(self.ui.tableView_sequence))
        print(self.sequence)

    def remove_last(self):
        if self.sequence['index'] > 0:
            self.sequence['index']-=1
            for key in self.sequence_keys:
                self.sequence[key][self.sequence['index']]=''
            self.refresh_table()
        else:
            pass
    def refresh_terminal(self,data):
        self.ui.textBrowser_terminal.append(data)

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
        i = 0
        data_string = ''
        for i in range(self.NITEMS):
            direction,degrees,pause=tuple([self.sequence[key][i] for key in self.sequence_keys])
            if direction == '' or degrees=='' or pause=='':
                print('skipping empty move')
                continue
            direction=self.stepper.dict_directions[direction]
            # Assume direction,degrees,pause order
            data = f'{str(direction)},{str(degrees)},{str(pause)};'
            data_string += data
        data_string+='$'
        #self.serial.send_data(data_string)
        print(data_string)

    def read_data(self):
        self.log("reading...")

    def refresh(self):
        self.serial.refresh_ports()
        self.ui.comboBox_port.clear()
        self.ui.comboBox_port.addItems(self.serial.list_ports)

    def clear(self):
        self.ui.textBrowser_terminal.clear()

if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = TheApp()
    window.show()
    sys.exit(app.exec_())
