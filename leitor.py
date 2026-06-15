from flask import Flask, render_template
from flask_socketio import SocketIO
import serial
import json
import threading
import re

app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*")

users = {
    "319EF3": "Dimitri Garaluz",
}


def read_serial():
    arduino = serial.Serial("/dev/ttyUSB0", 115200)

    while True:
        if arduino.in_waiting > 0:
            line = arduino.readline().decode("utf-8").strip()

            if '"tag":' in line:
                try:
                    start = line.index("{")
                    end = line.rindex("}") + 1
                    json_str = line[start:end]
                    json_str = re.sub(r'("tag":\s*)([A-Fa-f0-9]+)', r'\1"\2"', json_str)

                    data = json.loads(json_str)
                    tag_id = data["tag"]

                    if tag_id in users:
                        socketio.emit(
                            "new_scan", {"name": users[tag_id], "status": "approved"}
                        )
                    else:
                        socketio.emit(
                            "new_scan", {"name": "Unknown", "status": "denied"}
                        )

                except Exception:
                    pass


@app.route("/")
def index():
    return render_template("index.html")


if __name__ == "__main__":
    threading.Thread(target=read_serial, daemon=True).start()
    socketio.run(app, port=5000)
