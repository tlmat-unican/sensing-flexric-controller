from time import sleep, time_ns
import numpy as np
import e2connection
import struct
import csv
import os


def main():
   
    e2conn = e2connection.E2Connection("0.0.0.0", 12345)
    e2conn.tcp_connect()

    # Cargar los datos base
    z_init = np.loadtxt('sensing_data_z_init.csv', delimiter=',')
    r = np.loadtxt('sensing_data_r_init.csv', delimiter=',')
    theta = np.loadtxt('sensing_data_th_init.csv', delimiter=',')

    e2conn.start_receiving()


    active_params = e2connection.get_control_params()

    log_file = "tx_log.csv"
    file_exists = os.path.isfile(log_file)
    with open(log_file, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)
        
        writer.writerow(["timestamp_ns", "packet_size_bytes"])

    print("Iniciando con configuración por defecto:", active_params)
    e2conn.start_receiving()
    try:
        while True:
            ctrl_params = e2connection.get_control_params()

            
            if ctrl_params["flagtoread"]:
                print(f"Nuevo comando recibido: {ctrl_params}")
                active_params = ctrl_params.copy()
                e2connection.eflagCtrlread(isDatatoread=False)

          
            period = active_params["period"]
            precision = active_params["resolution"]

           
            z_org = np.loadtxt('sensing_data_z.csv', delimiter=',')
            z_org = e2connection.process_heatmap(z_org, r, theta)
            rows, cols = z_org.shape
           
            headerFmt = '<6fi'
            headerdata = struct.pack(
                headerFmt,
                active_params["theta_min"],
                cols,
                active_params["theta_max"],
                active_params["r_min"],
                rows,
                active_params["r_max"],
                precision
            )

            # Ajustar precisión
            if precision == 0:
                z = z_org.astype(np.int32)
                dtype_char = 'i'
            elif precision == 1:
                z = z_org.astype(np.float32)
                dtype_char = 'f'
            else:
                z = z_org.astype(np.float64)
                dtype_char = 'd'

            # Empaquetar matriz
            dataFmt = f'<{z.size}{dtype_char}'
            sensing_data = struct.pack(dataFmt, *z.flatten(order='C'))
            sensing_data_full = headerdata + sensing_data

            # Empaquetar tamaño + datos
            sensing_data_full = struct.pack('<i', len(sensing_data_full)) + sensing_data_full

            # Enviar
            e2conn.send_sensing_data(sensing_data_full)

            
            timestamp_ns = time_ns()
            packet_size = len(sensing_data_full)
            with open(log_file, 'a', newline='') as csvfile:
                writer = csv.writer(csvfile)
                writer.writerow([timestamp_ns, packet_size])

            print(f"Enviado ({packet_size} bytes) "
                  f"period={period}s, precision={precision}, "
                  f"theta=[{active_params['theta_min']},{active_params['theta_max']}], "
                  f"r=[{active_params['r_min']},{active_params['r_max']}]"
                  f"ncols {cols}, nrows {rows}")

            sleep(period)

    except KeyboardInterrupt:
        print("Interrumpido por el usuario.")
    except Exception as E:
        raise E
    finally:
        print("FINALLY")


if __name__ == "__main__":
    main()
