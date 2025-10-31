import socket
import struct
import time
import subprocess
import csv
import numpy as np
import sys


class PID:
    def __init__(self, Kp, Ki, Kd, setpoint, initial_value):
        self.Kp = Kp
        self.Ki = Ki
        self.Kd = Kd
        self.setpoint = setpoint
        self.integral = 0
        self.prev_error = 0
        self.output = initial_value

    def update(self, measurement, dt):
        error = self.setpoint - measurement

        self.integral += error * dt
        print(error*dt)
        derivative = (error - self.prev_error) / dt 
        change = self.Kp * error + self.Ki * self.integral + self.Kd * derivative
        self.output += change
        self.prev_error = error
        return self.output


HOST = '192.155.0.10'
PORT = 5000
IFACE = 'lo'
TARGET_DELAY = 10        # ms
INITIAL_RATE = 15      # Mbps
LOG_FILE = "pid_log.csv"


if len(sys.argv) > 1:
    PORT = int(sys.argv[1])

pid = PID(Kp=-1, Ki=0, Kd=0, setpoint=TARGET_DELAY, initial_value=INITIAL_RATE)



def recv_all(sock, length):
    data = b''

    while len(data) < length:
        packet = sock.recv(length - len(data))
        if not packet:
            return None
        data += packet
    return data





with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((HOST, PORT))
    print(f"Conectado al servidor {HOST}:{PORT}")
    print(f"Tasa inicial: {INITIAL_RATE} Mbps")

    msg_counter = 0
    last_time = time.time()

 
    while True:
    
        header_data = recv_all(s, 4)
        if not header_data:
            break
        msg_len = struct.unpack('!I', header_data)[0]
        msg = recv_all(s, msg_len)
        if not msg:
            break

        t_send = struct.unpack('!d', msg[:8])[0]

        t_recv = time.time_ns()
        e2e_delay = (t_recv - t_send) /1e6  # ms


        dt = (t_recv - last_time)/1e9
        last_time = t_recv


        pid_rate = pid.update(e2e_delay, dt)
      


   
        print(f"[{msg_counter:03d}] Delay: {e2e_delay} ms | dt {dt:.3f}s")
        msg_counter += 1
