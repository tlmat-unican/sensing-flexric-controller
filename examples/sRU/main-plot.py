# from jcasdata import JcasData
# from dual_mode_demo.parameters_settings import *
from time import sleep
import polar_plotter as myplot
import numpy as np
from matplotlib import pyplot as plt, animation, colors
import csv
import e2connection 
import struct


def main():
    # # init sensing-data fetcher
    # fetcher = JcasData() # config see dual_mode_demo/parameters_settings.py
  
    # # dict with fields {"th": th, "r": r, "z": z}
    # data  = fetcher.init_data # get "empty" data e.g. for initializing a UI with
    
    # # unpack data from the dict
    # theta = data['th']
    # r     = data['r']
    # z     = data['z']

    e2conn = e2connection.E2Connection("0.0.0.0", 12345)
    e2conn.tcp_connect()


    z = np.loadtxt('sensing_data_z_init.csv',  delimiter=',')
    r = np.loadtxt('sensing_data_r_init.csv', delimiter=',')
    theta = np.loadtxt('sensing_data_th_init.csv', delimiter=',')
    # init polar plotter
    # print(z.transpose(), r[0, 0], r[0, -1], theta[0, 0]/np.pi*180, theta[-1, 0]/np.pi*180, 0, r[0, -1], 63, 'Greys')
    # handles = myplot.init_polar_heatmap(z.transpose(), r[0, 0], r[0, -1], theta[0, 0]/np.pi*180, theta[-1, 0]/np.pi*180, 0, r[0, -1], 256//2, 63, 'Greys') 
    e2conn.start_receiving()
    try:
        # data fetching loop
        while True:
            global control_params
            # fetch the data (ready for plot)
            # data = fetcher.fetch()
            # plot sensing data in a polar plot
            z_org = np.loadtxt('sensing_data_z.csv', delimiter=',')
            ## Con esto cambio totalmenet la precision de la matriz
                    
          
      
     
            
            ctrl_params = e2connection.get_control_params()
            if ctrl_params["flagtoread"]:
                print("process_heatmap")
                z = e2connection.process_heatmap(z_org, r, theta)
                timeperiod = ctrl_params['period']
                precision = ctrl_params['resolution']
                headerFmt = '<6fi'
                headerdata = struct.pack(headerFmt, ctrl_params["theta_min"], 256//2, ctrl_params['theta_max'], r[0,0], 63, r[0,-1], precision)
            else:
                precision = 1
                timeperiod = 1
                z = z_org
                headerFmt = '<6fi'
                headerdata = struct.pack(headerFmt, theta[0,0]/np.pi*180, 256//2, theta[-1,0]/np.pi*180, r[0,0], 63, r[0,-1], precision)
            # 'i' para int32, 'f' para float32, 'd' para float64  -> pensar otra manera que sea mas elegante
            if precision == 0:
                z = z.astype(np.int32)
                dtype_char = 'i'
            if precision == 1:
                z = z.astype(np.float32)
                dtype_char = 'f'
            elif precision == 2:
                z = z.astype(np.float64)
                dtype_char = 'd'
            print("Shape:", z.shape)
            
            # myplot.update_polar_heatmap(handles, z.transpose(), [])
            # plt.savefig('polar_plot.png')
            # print("Z size before sending: ", z.nbytes, ", also shape: ", z.shape, " and dtype: ", z.dtype)
            
            # headerdata = struct.pack(headerFmt, theta[0,0]/np.pi*180, 4, theta[-1,0]/np.pi*180, r[0,0], 4, r[0,-1], precision)
            # print("Header bytes reales:", len(headerdata))  
            dataFmt = f'<{z.size}{dtype_char}'
                # matrix = np.arange(1, 17).reshape(4, 4).astype(np.float64)
                # dataFmt = f'<{matrix.size}{dtype_char}'
                # print(matrix)
            sensing_data = struct.pack(dataFmt, *z.flatten(order='C'))
            sensing_data_full = headerdata + sensing_data
            # print(f"Sending sensing data of length {len(sensing_data_full)} bytes")
            sensing_data_full = struct.pack('<i', len(sensing_data_full)) + sensing_data_full
            # print("[SM NEW]: Raw binary dump (hex, 8 bytes per line):")
            # for i in range(0, len(sensing_data_full), 8):
            #     line_bytes = sensing_data_full[i:i+8]
            #     print(' '.join(f'{b:02X}' for b in line_bytes))
            e2conn.send_sensing_data(sensing_data_full)
            e2connection.eflagCtrlread(isDatatoread=False)

            sleep(timeperiod) # see what frequency works for your setup
            # break
    except KeyboardInterrupt:
        # fetcher.fetcher_cleanup() # closes socket connection
        quit()
    except Exception as E:
        # fetcher.fetcher_cleanup()
        raise E
    finally:
        # if handles and plt.fignum_exists(handles["fig"].number):
        #     print("Test finished. Close plot window to exit.")
        #     plt.show() # Keep open
        # myplot.close_plot(handles) # Ensure ioff is called

        print("FINALLY")
if __name__ == "__main__":
    main()
