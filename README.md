
# Unican-dev

This repository provides a suite of tools for emulation and testing within the O-RAN Alliance framework. It includes E2 node emulators, a Near-RT RIC, and various xApps.

## Core Components: FlexRIC

This project is built upon **FlexRIC**, which provides the foundational components for the O-RAN architecture.

- **Compliance**: Adheres to the [O-RAN Alliance](https://www.o-ran.org/) specifications.
- **Implemented Service Models**:
  - O-RAN Standards: KPM v2, KPM v3, and RC.
  - Custom Models: NG/GTP, PDCP, RLC, MAC, SLICE, TC.
- **Encoding**: Supports multiple schemes such as ASN.1, FlatBuffers, and plain text, depending on the service model.
- **Data Storage**: The xApps use SQLite3 or MySQL to store indication data, facilitating offline processing and analysis (e.g., for AI/ML applications).

For further details on installation and operation, please refer to the original [OAI-FlexRIC](https://gitlab.eurecom.fr/mosaic5g/flexric) repository.

## :microscope: Sensing Service Model

This implementation focuses on a service model designed for collecting sensing data.

- **Location**: The source code is located in the [`./src/sm/new_sm/`](./src/sm/new_sm/) folder.
- **Information Element (IE)**: The predefined data unit used to gather information is defined in the [`ie/new_data_ie.h`](./src/sm/new_sm/ie/new_data_ie.h) file.

## Running the Components

This section details the commands to execute each component of the system.

> [!IMPORTANT]
> All configuration files are located in the [`conf_files/`](./conf_files/) directory. To set the IP address, edit the corresponding configuration file and update the `NearRT_RIC_IP` field.

### 1. Near-RT RIC

To launch the Near-Real-Time RIC, run the following command:

```bash
./build/examples/ric/nearRT-RIC -c ./conf_files/ric.conf
```

### 2. Monitoring xApp

The example xApp is located at [`/examples/xApp/c/monitor/xapp_new_moni.c`](./examples/xApp/c/monitor/xapp_new_moni.c). Use the provided configuration file to define the database access attributes and the service model to be used.

To launch the xApp:

```bash
./build/examples/xApp/c/monitor/xapp_sense_moni -c ./conf_files/xapp_new_sm.conf
```

### 3. E2 Agent

The E2 agent ([`test_agent3.c`](./examples/emulator/agent/test_agent2.c)) receives sensing information from the processing unit, parses it, and makes it available to the Near-RT RIC.

To launch the agent:

```bash
./build/examples/emulator/agent/emu_agent_new -c ./conf_files/e2agent.conf
```

## :satellite: sensing Radio Unit Emulator

The [`/examples/sRU/`](./examples/sRU/) directory contains two sensing data emulators: `test_pid.py` and `test_ctrl.py`.

These programs load a **heatmaps** generated from processed Estimated Channel Coefficients (CHEs). The heatmap resolution is defined by the number of angles (`n_angles`) and ranges (`n_ranges`), resulting in an `n_angles × n_ranges` matrix where each pixel represents the **signal strength**.

---

## :test_tube: Use Cases and Validation

### Control Loop (CTRL) Validation

To validate the operation of the control loop, follow these steps:

1.  **Launch the E2 Agent**
    ```bash
    ./build/examples/emulator/agent/emu_agent_new -c ./conf_files/e2agent.conf
    ```
2.  **Launch the sRU Emulator**
    ```bash
    python3 examples/sRU/test_ctrl.py
    ```
3.  **Launch the Control xApp**
    ```bash
    ./build/examples/xapp_cust_sense_moni_ctrl_cmd_test -c ./conf_files/xapp_new_sm.conf
    ```
4.  **Analyze the Results**
    Use the [Jupyter Notebook](./examples/sRU/ctrl_rate_inst.ipynb) to visualize and analyze the results of this use case.

### PID Controller Validation

#### Scenario 1: Ensuring a Target E2E Delay

To validate the PID controller's performance in allocating capacity to achieve a target delay, follow these steps:

1.  **Launch the E2 Agent**
    ```bash
    ./build/examples/emulator/agent/emu_agent_new -c ./conf_files/e2agent.conf
    ```
2.  **Launch the sRU Emulator**
    ```bash
    python3 examples/sRU/test_pid.py
    ```
3.  **Launch the xApp with PID**
    ```bash
    # Usage: ./executable -c <conf_file> <target_delay_ms> <Kp> <T_change_ms>
    ./build/examples/xapp_cust_sense_moni_ctrl_cmd_slot -c ./conf_files/xapp_new_sm.conf 80 1 1000
    ```
4.  **(Optional) Generate Background Traffic**
    Run `client_bk.py` and `server_bk.py` from the [`test-PID/`](./test-PID/) directory to simulate background traffic.

5.  **Run the Network Controller**
    Launch the [controller](./test-PID/e2_control.py) to apply configuration changes through the interface.

6.  **Analyze the Results**
    Open the [Jupyter Notebook](./test-PID/pid.ipynb) to visualize and analyze the results of the PID controller's capacity allocation.

#### Scenario 2: Dynamic E2E Target Delay

In this scenario, the target delay is dynamically updated every 20 seconds. Repeat steps 1, 2, 5, and 6 from the previous scenario, but use the following xApp instead:

```bash
./build/examples/xapp_cust_sense_moni_ctrl_cmd_slot_target -c ./conf_files/xapp_new_sm.conf
```

## :envelope_with_arrow: Contact

-   [Fátima Khan Blanco (khanf@unican.es)](mailto:khanf@unican.es)
-   [Luis Diez (diezlf@unican.es)](mailto:diezlf@unican.es)
-   [Ramón Agüero (agueroc@unican.es)](mailto:agueroc@unican.es)