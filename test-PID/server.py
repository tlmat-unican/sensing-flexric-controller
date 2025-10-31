import socket
import struct
import time

import sys


HOST = '0.0.0.0'
PORT = 5000

print("Total arguments:", len(sys.argv))



if len(sys.argv) > 1:
    PORT = int(sys.argv[1])

print(f"Usando HOST={HOST}, PORT={PORT}")
    
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    s.bind((HOST, PORT))
    s.listen(1)
    print(f"Servidor enviando datos en {HOST}:{PORT}")
    conn, addr = s.accept()
    prev_send = 0
    with conn:
        print(f"Conectado con {addr}")
        i = 0
            
        start = time.time()
        while True:
            payload =  b'A' * 64512  
            t_send = time.time_ns()
            msg = struct.pack('!d', t_send) + payload
            header = struct.pack('!I', len(msg))
           
            
            conn.sendall(header + msg)
            
            if prev_send!=0:
                interval = (t_send - prev_send)/1e9
          
                elapsed = time.time() - start
                print(f"[{i:03d}] Elapsed Time: {elapsed:2f} Rate: {(len(msg)*8/interval)/1e6:.2f}Mbps || Interval {interval:.2f} s")
            prev_send = t_send
            i += 1
            time.sleep(0.05)
