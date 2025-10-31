# Unican-dev



## FlexRIC

This repository contains [O-RAN Alliance](https://www.o-ran.org/) compliant E2 node Agent emulators, a NearRT-RIC, xApps written in C/C++, Python and Go. 
It implements O-RAN service models (KPM v2, KPM v3, and RC) and various customized service models (NG/GTP, PDCP, RLC, MAC, SLICE, TC) and a built-in emulation.
Depending on the service model, different encoding schemes have been developed (ASN.1, flatbuffer, plain).
The indication data received in the xApp uses SQLite3 or MySQL database to store the data for enabling offline processing applications
(e.g., ML/AI). 

For further details and installation instructions, please refer to the [OAI-FlexRIC](https://gitlab.eurecom.fr/mosaic5g/flexric) repository. 


## Testing [UNICAN]
This section focuses on the onworking service model, which is responsible for collecting sensing data. The implementation of this service model can be found in the [new_sm](./src/sm/new_sm/) folder.  

Within this model, the information element (IE), the predefined unit of data used to gather information, is defined in [ie/new_data_ie.h](./src/sm/new_sm/ie/new_data_ie.h)



Commands to execute each component:

- **RIC**
  
  To launch the nearRT-RIC:
  ```bash
  ./build/examples/ric/nearRT-RIC -c ./conf_files/ric.conf
  ```

- **xAPP**  
  To launch the monitoring [xApp](./examples/xApp/c/monitor/xapp_new_moni.c), use the provided [configuration file](./conf_files/xapp_new_sm.conf).  
  This configuration file specifies the required attributes to access the database as well as the SM of interest.

  ```bash
  ./build/examples/xApp/c/monitor/xapp_new_moni -c ./conf_files/xapp_new_sm.conf
  ```

- **E2 Agent**

  To launch the [E2 Agent](./examples/emulator/agent/test_agent2.c), run:
  ```bash
  ./build/examples/emulator/agent/emu_agent_new -c ./conf_files/e2agent.conf
  ```
  This agent is responsible for receiving the sensing information from the processing unit, parsing it, and making it available to the near-RT RIC.


## **sRU Emulator**

Two different sensing data emulators are available in the [sRU](./examples/sRU/) directory: `test_pid.py` and `test_ctrl.py`.

These programs load **heatmaps** generated from processed Estimated Channel Coefficients (CHEs).  
The heatmap resolution is defined by the number of angles (`n_angles`) and ranges (`n_ranges`), resulting in a total of `n_angles × n_ranges` pixels, each representing the **signal strength**.



### **CTRL Validation**
To validate the control loop, execute the following components:
Execute the E2 agent + test_ctrl.py + xapp_cust_sense_moni_ctrl_cmd_test.c 

1. **E2 Agent**  
  ```bash
  ./build/examples/emulator/agent/emu_agent_new -c ./conf_files/e2agent.conf
  ```
2. **sRU  Emulator**
  ```bash
  python3 examples/sRU/test_ctrl.py
  ```
3. **xApp**
  ```bash
  ./build/examples/xapp_cust_sense_moni_ctrl_cmd_test -c ./conf_files/xapp_new_sm.conf
  ```
4. **Analyze results**
  To visualize and analyze the results from the control use case, open the following [jupyter notebook](./examples/sRU//ctrl_rate_inst.ipynb)


### **PID Controller**

#### **Ensuring a target E2 end-to-end delay**

To validate the PID controller performance in allocating capacity to achieve a target delay, please follow these steps:

1. **E2 Agent**  
  ```bash
  ./build/examples/emulator/agent/emu_agent_new -c ./conf_files/e2agent.conf
  ```
2. **sRU  Emulator**
  ```bash
  python3 examples/sRU/test_pid.py
  ```

3. **xApp**
  ```bash
  ./build/examples/xapp_cust_sense_moni_ctrl_cmd_slot -c ./conf_files/xapp_new_sm.conf <del_target_ms> <Kp> <T_change_ms>
  ```
4. **Sensing background traffic** 
  To execute background traffic execute client_bk.py and server_bk in [test-PID](./test-PID/).

5. **Network Controller**
 Run the [controller](./test-PID/e2_control.py) to apply configuration changes through the interface.


6. **Analyze results**
  To visualize and analyze the results from the PID controller alocation capacity, open the following [jupyter notebook](./test-PID/pid.ipynb)

#### **Dynamic Target for E2 End-to-End Delay**

In this scenario, repeat steps 1, 2, 5, and 6.  
However, instead of the previous xApp, run the following xApp, which dynamically updates the target every 20 seconds:

```bash
./build/examples/xapp_cust_sense_moni_ctrl_cmd_slot_target -c ./conf_files/xapp_new_sm.conf



```
> [!NOTE]  
> All configuration files are located in the [`conf_files`](./conf_files/) directory.  
> To set the IP address, edit the corresponding configuration file and update the `NearRT_RIC_IP` field.  
> In the xApp configuration file, you can also modify the update frequency to adjust how often data is sent or processed.
```


## :envelope_with_arrow:  Contact 
* [Fátima Khan Blanco (khanf@unican.es)](mailto:khanf@unican.es)
* [Luis Diez (diezlf@unican.es)](mailto:diezlf@unican.es)
* [Ramón Agüero (agueroc@unican.es)](mailto:agueroc@unican.es)
