import sys
from PyQt5.QtWidgets import QMainWindow,QApplication
from terminal_gui import Ui_MainWindow


class TheApp(QMainWindow):
    def __init__(self):
        super().__init__() # Heredar atributeos de QMainWindow
        self.ui = Ui_MainWindow()
        self.ui.setupUi(self)

if __name__ == '__main__':
    app = QApplication()
    window = TheApp()
    window.show()
    sys.exit(app.exec__())
