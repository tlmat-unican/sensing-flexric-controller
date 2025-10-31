import socket
import subprocess
import struct

UDP_IP = "192.255.0.10"
UDP_PORT = 12345
INTERFACE = "enp100s0"  # network interface -> used by E2 -> not the one where we are listening 
# UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
sock.bind((UDP_IP, UDP_PORT))

print(f"Listening for UDP commands on {UDP_IP}:{UDP_PORT}")




def apply_tc(command):
    try:
        print(f"[CMD] {command}")
        subprocess.run(command, shell=True, check=True)
    except subprocess.CalledProcessError as e:
        print(f"Error running tc: {e}")

tc_cmd = f"sudo tc qdisc replace dev {INTERFACE} root netem rate 10Mbit"
apply_tc(tc_cmd)

while True:
    data, addr = sock.recvfrom(8)  # 8 bytes = tamaño de un double
    if len(data) < 8:
        print(f"Ignored short packet ({len(data)} bytes) from {addr}")
        continue

    rate = struct.unpack('d', data)[0] 
    if  1 < rate <= 1000:
        print(f"Rate válido: {rate:.2f} Mbps")
        tc_cmd = f"sudo tc qdisc replace dev {INTERFACE} root netem rate {rate:.4f}Mbit"
        apply_tc(tc_cmd)

        

    
