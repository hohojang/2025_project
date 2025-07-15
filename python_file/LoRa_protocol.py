import sys
import serial
from PyQt5.QtWidgets import QApplication, QWidget, QPushButton, QVBoxLayout, QLabel, QComboBox

class LoRaConfigurator(QWidget):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("E220-900T22D Configurator")

        # 시리얼 포트 설정
        self.ser = serial.Serial('COM3', 9600, timeout=1)  # 포트 번호 및 속도 확인 필요

        layout = QVBoxLayout()

        # 주소 설정
        self.addr_label = QLabel("주소 설정:")
        self.addr_combo = QComboBox()
        for i in range(1, 16):
            self.addr_combo.addItem(f"{i} (0x{i:04X})")
        self.addr_button = QPushButton("주소 설정")
        self.addr_button.clicked.connect(self.set_address)

        # 채널 설정
        self.chan_label = QLabel("채널 설정:")
        self.chan_combo = QComboBox()
        for i in range(1, 16):
            self.chan_combo.addItem(f"{i} (0x{i:02X})")
        self.chan_button = QPushButton("채널 설정")
        self.chan_button.clicked.connect(self.set_channel)

        # 통신속도 설정
        self.baud_label = QLabel("통신속도 설정:")
        self.baud_combo = QComboBox()
        self.baud_rates = {
            "1200bps": "0",
            "2400bps": "1",
            "4800bps": "2",
            "9600bps": "3",
            "19200bps": "4",
            "38400bps": "5",
            "57600bps": "6",
            "115200bps": "7"
        }
        for k in self.baud_rates.keys():
            self.baud_combo.addItem(k)
        self.baud_button = QPushButton("통신속도 설정")
        self.baud_button.clicked.connect(self.set_baudrate)

        # UI 정렬
        layout.addWidget(self.addr_label)
        layout.addWidget(self.addr_combo)
        layout.addWidget(self.addr_button)
        layout.addWidget(self.chan_label)
        layout.addWidget(self.chan_combo)
        layout.addWidget(self.chan_button)
        layout.addWidget(self.baud_label)
        layout.addWidget(self.baud_combo)
        layout.addWidget(self.baud_button)

        self.setLayout(layout)

    def send_command(self, cmd):
        print(f"전송: {cmd.strip()}")
        self.ser.write((cmd + '\r\n').encode())

    def set_address(self):
        selected_index = self.addr_combo.currentIndex() + 1
        hex_addr = f"{selected_index:04X}"
        self.send_command(f"AT+ADDR={hex_addr}")

    def set_channel(self):
        selected_index = self.chan_combo.currentIndex() + 1
        hex_chan = f"{selected_index:02X}"
        self.send_command(f"AT+CHAN={hex_chan}")

    def set_baudrate(self):
        selected_text = self.baud_combo.currentText()
        val = self.baud_rates[selected_text]
        self.send_command(f"AT+UART=8,{val},1")  # 8N1 기준

if __name__ == "__main__":
    app = QApplication(sys.argv)
    window = LoRaConfigurator()
    window.show()
    sys.exit(app.exec_())
