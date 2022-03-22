import sys
from PyQt5.QtWidgets import QMainWindow,QApplication
from terminal_gui import Ui_MainWindow


class TheApp(QMainWindow):
    def __init__(self):
        super().__init__() # Heredar atributeos de QMainWindow
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)

        # EVENTS

        self.ui.btn_connect.clicked.connect(self.connect)
        self.ui.btn_send.clicked.connect(self.send_data)
        self.ui.btn_refresh.clicked.connect(self.refresh)
        self.ui.btn_clear.clicked.connect(self.clear)
    def connect(self):
        print('connecting...')

    def send_data(self):
        print('sending...')
    
    def read_data(self):
        print("reading...")

    def refresh(self):
        print("refreshing...")

    def clear(self):
        print("cleaning...")


if __name__ == '__main__':
    app = QApplication(sys.argv)
    window = TheApp()
    window.show()
    sys.exit(app.exec_())
