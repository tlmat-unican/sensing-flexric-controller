#!/usr/bin/env python3
import subprocess
import time
import random
import json

def run_iperf(server, duration, rate, udp=True, parallel=1, output=None):
    cmd = [
        "iperf3", "-c", server,
        "-t", str(duration),
        "-P", str(parallel),
        "--interval", "1",
        "-J"
    ]
    if udp:
        cmd.append("-u")
        cmd += ["-b", str(rate)]

    print(f"\n Exec: {' '.join(cmd)}")
    result = subprocess.run(cmd, capture_output=True, text=True)
    
    if result.returncode != 0:
        print(f"Error: {result.stderr}")
    else:
        if output:
            with open(output, "a") as f:
                f.write(result.stdout + "\n")
        print("Test complet")

def constant_traffic(server):
    rate = input("CBR (ej: 10M): ")
    duration = int(input("Duartion (s): "))
    udp = input("¿UDP? (y/n): ").lower() == "y"
    run_iperf(server, duration, rate, udp)

def burst_traffic(server):
    rates = input("Lista de tasas separadas por coma (ej: 1M,5M,10M): ").split(",")
    burst_time = int(input("Duración por ráfaga (s): "))
    gap = float(input("Pausa entre ráfagas (s): "))
    udp = input("¿UDP? (y/n): ").lower() == "y"
    for r in rates:
        run_iperf(server, burst_time, r.strip(), udp, output="burst_results.json")
        time.sleep(gap)

def ramp_traffic(server):
    start = int(input("Tasa inicial (Mbps): "))
    end = int(input("Tasa final (Mbps): "))
    step = int(input("Incremento (Mbps): "))
    duration = int(input("Duración por paso (s): "))
    udp = input("¿UDP? (y/n): ").lower() == "y"
    for rate in range(start, end + step, step):
        run_iperf(server, duration, f"{rate}M", udp, output="ramp_results.json")

def onoff_traffic(server):
    mean_on = float(input("Duración media de ON (s): "))
    mean_off = float(input("Duración media de OFF (s): "))
    rate = input("Tasa durante ON (ej: 10M): ")
    total_time = float(input("Duración total (s): "))
    udp = input("¿UDP? (y/n): ").lower() == "y"

    elapsed = 0.0
    while elapsed < total_time:
        on_time = random.expovariate(1.0 / mean_on)
        off_time = random.expovariate(1.0 / mean_off)

        if elapsed + on_time > total_time:
            on_time = total_time - elapsed

        print(f"\n ON durante {on_time:.2f}s a {rate}")
        run_iperf(server, int(on_time), rate, udp, output="onoff_results.json")
        elapsed += on_time

        if elapsed >= total_time:
            break

        print(f" OFF durante {off_time:.2f}s")
        time.sleep(off_time)
        elapsed += off_time

def main():
    print("=== iperf3 Traffic Generator ===")
    server = input("IP iperf3 server: ")

    modes = {
        "1": ("CBR", constant_traffic),
        "2": ("Bursty", burst_traffic),
        "3": ("Ramp", ramp_traffic),
        "4": ("ON/OFF", onoff_traffic)
    }

    print("\nTypos de traffic:")
    for k, (desc, _) in modes.items():
        print(f"  {k}) {desc}")

    choice = input("\nSelecciona type: ").strip()
    if choice not in modes:
        print("No available choice.")
        return

    _, func = modes[choice]
    func(server)

if __name__ == "__main__":
    main()
