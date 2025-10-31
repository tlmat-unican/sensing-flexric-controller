#include <stdatomic.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "xApp/e42_xapp_api.h"
#include "util/alg_ds/alg/defer.h"
#include "util/time_now_us.h"
#include "sm/new_sm/new_sm_id.h"

// -----------------------------
// PID Controller
// -----------------------------
typedef struct {
    double Kp;
    double Ki;
    double Kd;
    double prev_error;
    double integral;
} pid_controller_t;

void pid_init(pid_controller_t* pid, double Kp, double Ki, double Kd) {
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->prev_error = 0;
    pid->integral = 0;
}

double pid_compute(pid_controller_t* pid, double setpoint, double measured, double dt) {

    double error = setpoint - measured;
    pid->integral += error * dt; //necesitamos acumulado -> cuidado windup
    if (pid->integral > 1000) printf("Pid->integral windup");
    double derivative = (error - pid->prev_error) / dt;
    pid->prev_error = error; 
    return pid->Kp * error + pid->Ki * pid->integral + pid->Kd * derivative;
}

// -----------------------------
// Variables globales
// -----------------------------
static volatile bool exit_flag = false;
static int64_t prev_time = 0;
static int64_t last_time_us = 0;
static int64_t last_time_cb_us = 0;
static double reg_time_btw_cb = 0;
static int64_t last_tc_change_us = 0;
static FILE* latencyfile = NULL;
static FILE* ratefile = NULL;
_Atomic uint16_t assoc_rnti = 0;
static pid_controller_t pid_ctrl;

// Configuración por defecto (ajustables por consola)
double latency_ref_ms = 5.0;           // objetivo latencia
static double dt_default = 0.001;             // periodo de muestreo
static int64_t min_tc_interval_us = 200000;   // 200 ms entre invocaciones a tc
static const double rate_min_mbps = 10.0; // 10 Mbps min -> para que no me casque el loopback 
static const double rate_max_mbps = 30000.0; // 10 Gbps max 
static const double min_rate_delta_mbps = 0.5;
static double current_rate_mbps = 0; // starting rate

// -----------------------------
static void sigint_handler(int sig) {
    printf("Signal %d received! Exiting...\n", sig);
    exit_flag = true;
}


static void sm_cb_new(sm_ag_if_rd_t const* rd, global_e2_node_id_t const* e2_node) {
    if (!rd || !e2_node) return;
    assert(rd->type == INDICATION_MSG_AGENT_IF_ANS_V0);
    assert(rd->ind.type == NEW_STATS_V0);

    int64_t now = time_now_us();
    int64_t latency_us = now - rd->ind.new_sm.msg.tstamp;
    double latency_ms = (double) (latency_us / 1000.0);
    reg_time_btw_cb = (double) (now - last_time_cb_us)/1000; // just in case we want to treat the subs as an error
    
    // printf("Time btw calls: %.2f \n", reg_time_btw_cb);
    last_time_cb_us = now;
    if (prev_time == rd->ind.new_sm.msg.tstamp) // (HEATMAP viejo)
        return;
    // if (latency_ms > reg_time_btw_cb) latency_ms = latency_ms - reg_time_btw_cb;
    prev_time = rd->ind.new_sm.msg.tstamp;

    
    fprintf(latencyfile, "%lld\t%f\n", (long long) now, latency_ms);
    fflush(latencyfile);

    printf("[INFO] New heatmap arrived, latency: %.2f ms\n", latency_ms);

    // Calcular dt  -> lo calculo en segundos  
    double dt = (last_time_us > 0) ? (now - last_time_us) / 1e6 : dt_default;
    if (dt <= 0.0) dt = dt_default;
    last_time_us = now;


    double pid_output = pid_compute(&pid_ctrl, latency_ref_ms, latency_ms, dt);
    double new_rate_mbps = current_rate_mbps - pid_output; // Inversamente proporcional al retardo

    printf("[PID] latency_ref: %f ms | latency_meas: %.2f ms | output: %.2f \n", latency_ref_ms, latency_ms, -pid_output);

    if (new_rate_mbps < rate_min_mbps) new_rate_mbps = rate_min_mbps;
    if (new_rate_mbps > rate_max_mbps) new_rate_mbps = rate_max_mbps;

    int64_t now_us = now;
    bool enough_time = (now_us - last_tc_change_us) >= min_tc_interval_us; // Para hacer que se cambia el canal cada x -> A1-> SMO CHANGE


    if (enough_time) {
        char cmd[256];
        snprintf(cmd, sizeof(cmd),
                 "sudo tc qdisc replace dev lo root netem rate %.2fmbit", new_rate_mbps);

        int ret = system(cmd);
        if (ret != 0) {
            fprintf(stderr, "[ERROR] tc qdisc replace failed (ret=%d). cmd: %s\n", ret, cmd);
            perror("tc qdisc replace");
        } else {
            last_tc_change_us = now_us;
            // printf("[PID] latency: %.2f ms | new rate: %.2f Mbps | diff. rate: %.2f\n",
            //        latency_ms, new_rate_mbps, new_rate_mbps - current_rate_mbps);
        }
        fprintf(ratefile,  "%lld\t%f\n", (long long) now, new_rate_mbps);
        current_rate_mbps = new_rate_mbps;
    }

}


int main(int argc, char *argv[]) {
  

    // if (argc >= 3) latency_ref_ms = atof(argv[3]);
 
    // if (argc >= 4) min_tc_interval_us = (int64_t)(atof(argv[4]) * 1000.0);
    // if (argc >= 5) dt_default = atof(argv[5]);

    printf("[CONFIG-PID] latency_ref_ms=%.3f ms | min_tc_interval_us=%.3f s | dt=%.6f s\n",
           latency_ref_ms, min_tc_interval_us / 1e6, dt_default);

    latencyfile = fopen("xappsensectrl_dels.txt", "w");
    if (!latencyfile) {
        perror("fopen latency file");
        return -1;
    }
    ratefile = fopen("xappsensectrl_chann.txt", "w");
    if (!ratefile) {
        perror("fopen rate file");
        return -1;
    }

    fr_args_t args = init_fr_args(argc, argv);
    defer({ free_fr_args(&args); });

    pid_init(&pid_ctrl, 0.20, 0.02, 0.1);

    init_xapp_api(&args);
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigint_handler);
    sleep(1);

    e2_node_arr_xapp_t nodes = e2_nodes_xapp_api();
    defer({ free_e2_node_arr_xapp(&nodes); });
    assert(nodes.len > 0);

    printf("Connected E2 nodes len = %d\n", nodes.len);
    sm_ans_xapp_t* sense_handle = calloc(nodes.len, sizeof(sm_ans_xapp_t));
    assert(sense_handle != NULL);

    for (size_t i = 0; i < nodes.len; ++i) {
        sense_handle[i] = report_sm_xapp_api(&nodes.n[i].id, SM_NEW_ID,
                                             (void*)args.sub_cust_sm[i].time,
                                             sm_cb_new);
        assert(sense_handle[i].success == true);
        sleep(2);
    }

    while(!exit_flag) {
        usleep(1000);
    }

    for (int i = 0; i < nodes.len; ++i)
        rm_report_sm_xapp_api(sense_handle[i].u.handle);
    free(sense_handle);
    fclose(latencyfile);

    while(try_stop_xapp_api() == false)
        usleep(1000);

    printf("Test xApp run SUCCESSFULLY\n");
    return 0;
}
