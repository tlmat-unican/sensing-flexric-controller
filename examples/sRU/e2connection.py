import socket
import threading
import struct
import numpy as np


control_params = {
    "theta_min": -45,
    "theta_max": 45,
    "r_min": 0,
    "r_max": 8.498,
    "period": 0.01,
    "resolution": 2,
    "flagtoread": False
}


def updateglobalctrlparams(tlvs_info, isDatatoread=False):
    global control_params
    try:
        for tlv in tlvs_info["tlvs"]:
    
            t = tlv["type"]
            v = tlv["value"]
            if t == 0:  # SENSE_CTRL_SELECT_ANGLE
                control_params["theta_min"] = v["theta_init"]
                control_params["theta_max"] = v["theta_end"]
            elif t == 1:
                control_params["r_min"] = v["r_init"]
                control_params["r_max"] = v["r_end"]
            
            elif t == 2:  # SENSE_CTRL_SELECT_PERIODICITY
                control_params["period"] = v["period"]
            elif t == 3:  # SENSE_CTRL_SELECT_RESOLUTION
                control_params["resolution"] = v["resolution"]
        control_params["flagtoread"] = isDatatoread
    except Exception as e:
        print(f"[updateglobalctrlparams] >> Error {e}")

def eflagCtrlread(isDatatoread=False):
    global control_params
    control_params["flagtoread"] = isDatatoread

def get_control_params():
    global control_params
    return control_params.copy() 

   
def process_heatmap(z, r, theta):
    global control_params
    theta_min = control_params["theta_min"]
    theta_max = control_params["theta_max"]
    
    r_min = control_params["r_min"]
    r_max = control_params["r_max"]

    try:
        print(f"theta_min: {theta_min} theta_max: {theta_max}")
        col_angles = np.linspace(theta[0,0]*180/np.pi, theta[-1,0]*180/np.pi, z.shape[1])

      
        col_mask = (col_angles >= theta_min) & (col_angles <= theta_max)
  
       
        row_radios = np.linspace(r[0,0], r[-1,0], z.shape[0])

       
        row_mask = (row_radios >= r_min) & (row_radios <= r_max)

       
        if np.all(r_min == r_max) or not np.any(row_mask):
            raise ValueError("Invalid r_min/r_max range or empty selection")

        # Filtra z según la máscara de filas
        z_filtered = z[row_mask, :][:, col_mask]
    except Exception as e:
        print(f"[process_heatmap] >> Error {e}")
        z_filtered = z
    print("z_filtered")
    return z_filtered

class E2Connection:
    def __init__(self, e2_ip, e2_port):
        self.e2_ip = e2_ip
        self.e2_port = e2_port
        self.socket = None
        self.is_connected = False
        self.running = False
        self.recv_thread = None

    def tcp_connect(self):
        try:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.connect((self.e2_ip, self.e2_port))
            self.is_connected = True
            self.running = True
            print(f"Connected to E2 at {self.e2_ip}:{self.e2_port}")
        except Exception as e:
            print(f"Failed to connect to E2: {e}")
            self.is_connected = False

    def send_sensing_data(self, sensing_data):
        """Envia datos una sola vez (sin bucle)."""
        if not self.is_connected:
            print("Not connected to E2.")
            return
        try:
            self.socket.sendall(sensing_data)
            print("Sensing data sent successfully.")
        except Exception as e:
            print(f"Failed to send sensing data: {e}")
            self.running = False
            
    def unpacking_ctrl_msg(self, data):
        offset = 0
        print(f"\nReceived {len(data)} bytes")

        # Hex dump
        for i in range(0, len(data), 8):
            line_bytes = data[i:i+8]
            print(' '.join(f'{b:02X}' for b in line_bytes))

        # Header
        sense_id_device = struct.unpack_from("<i", data, offset)[0]  # signed int
        offset += 4
        print(f"sense_id_device={sense_id_device}")

        # num_tlvs y msg_len total
        num_tlvs, msg_len = struct.unpack_from("<II", data, offset)  # unsigned int
        offset += 8
        print(f"num_tlvs={num_tlvs}, msg_len={msg_len}")

        tlvs = []

        # ---- TLVs ---- T = cmd; L = length, V = values
        for i in range(num_tlvs):
            if offset + 2 > len(data):
                print(f" Truncated TLV header at {offset}")
                break

            tlv_type, tlv_len = struct.unpack_from("<BB", data, offset)
            offset += 2

            if offset + tlv_len > len(data):
                print(f" Truncated TLV value at {offset}")
                break

            value_bytes = data[offset: offset + tlv_len]
            offset += tlv_len

            try:
            
                if tlv_type == 0:  # SENSE_CTRL_SELECT_ANGLE
                    theta_init, theta_step, theta_end = struct.unpack_from("<fff", value_bytes)
                    value = {
                        "theta_init": theta_init,
                        "theta_step": theta_step,
                        "theta_end": theta_end
                    }
                elif tlv_type == 1: # SENSE_CTRL_SELECT_RADIO
                    radio_init, radio_step, radio_end = struct.unpack_from("<fff", value_bytes)
                    value = {
                        "r_init": radio_init,
                        "r_step": radio_step,
                        "r_end": radio_end
                    }
                elif tlv_type == 2:  # SENSE_CTRL_SELECT_PERIODICITY
                    (period,) = struct.unpack_from("<f", value_bytes)
                    value = {"period": period}

                elif tlv_type == 3:  # SENSE_CTRL_SELECT_RESOLUTION
                    (resolution,) = struct.unpack_from("<B", value_bytes)
                    value = {"resolution": resolution}

            except Exception as e:
                print(f"[unpacking_ctrl_msg] error in TLVs")
              

            tlvs.append({
                "type": tlv_type,
                "len": tlv_len,
                "value": value
            })

            print(f"[TLV {i}] type={tlv_type} len={tlv_len} value={value}")

       
        result = {
            "num_tlvs": num_tlvs,
            "msg_len": msg_len,
            "tlvs": tlvs
        }
        updateglobalctrlparams(result)
        eflagCtrlread(True)
        return result

    def start_receiving(self, buffer_size=1024):
        if not self.is_connected:
            print("Not connected to E2.")
            return

        def recv_loop():
            while self.running:
                try:
                    print("Reading data...")
                    data = self.socket.recv(buffer_size)

                    if not data or len(data) == 0:
                        print("Connection closed by server or no data received.")
                        self.running = False
                        break  
                  
                    try:
                        _ = self.unpacking_ctrl_msg(data)
                        
                    except Exception as e:
                        print(f"[start_receiving] Error unpacking control message: {e}")

                except Exception as e:
                    print(f"Failed to receive commands: {e}")
                    self.running = False
                    break  

       
        self.recv_thread = threading.Thread(target=recv_loop, daemon=True)
        self.recv_thread.start()

    def close(self):
        """Cierra la conexión y detiene los hilos."""
        self.running = False
        if self.socket:
            try:
                self.socket.shutdown(socket.SHUT_RDWR)
            except Exception:
                pass
            self.socket.close()
            print("Connection closed.")
 
