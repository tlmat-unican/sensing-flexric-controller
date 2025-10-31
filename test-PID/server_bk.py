import socket
import struct
import time

import sys


HOST = '192.155.0.12'
PORT = 8888

print("Total arguments:", len(sys.argv))



if len(sys.argv) > 1:
    PORT = int(sys.argv[1])
    
p = [2, 1, 0.5, 0.3, 0.2]
times  = [0.05, 0.01, 0.01, 0.01] 
pay = 64512

print(f"Usando HOST={HOST}, PORT={PORT}")
    
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((HOST, PORT))
    s.listen(1)
    print(f"Servidor enviando datos en {HOST}:{PORT}")
    conn, addr = s.accept()
    prev_send = 0
    start = time.time()
    elapsed = 0
    with conn:
        print(f"Conectado con {addr}")
        i = 0
        while True:
        
        
        
            
            elapsed = time.time() - start
            phase = elapsed % 40  

            if phase < 20:
                payload = b'A' * int(pay)
                print("Big-pay")
            else:
                payload = b'A' * int(pay * 0.5)
                print("Small-pay")

            t_send = time.time_ns()
            msg = struct.pack('!d', t_send) + payload
            header = struct.pack('!I', len(msg))
            conn.sendall(header + msg)

            if prev_send != 0:
                interval = (t_send - prev_send) / 1e9
                print(f"Interval: {interval:.3f}s")
                # print(f"[{i:03d}] Rate: {(len(msg)*8/interval)/1e6:.2f} Mbps")

            prev_send = t_send
            i += 1

            time.sleep(0.05)