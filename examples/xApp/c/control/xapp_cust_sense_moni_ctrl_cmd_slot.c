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
#include <errno.h>
#include <inttypes.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "xApp/e42_xapp_api.h"
#include "util/alg_ds/alg/defer.h"
#include "util/time_now_us.h"
#include "sm/new_sm/new_sm_id.h"

#define UDP_PORT 12345
#define IP_ADDR "192.255.0.10"

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
    pid->integral += error * dt; // watch windup
    if (pid->integral > 1000) printf("Pid->integral windup\n");
    double derivative = (dt > 0.0) ? (error - pid->prev_error) / dt : 0.0;
    pid->prev_error = error;
    return pid->Kp * error + pid->Ki * pid->integral + pid->Kd * derivative;
}

// -----------------------------
// Globals
// -----------------------------
static volatile bool exit_flag = false;
static int64_t prev_time = 0;
static int64_t last_time_us = 0;
static int64_t last_time_cb_us = 0;
static double reg_time_btw_cb = 0;
int64_t last_tc_change_us = 0;
static FILE* latencyfile = NULL;
static FILE* ratefile = NULL;
static FILE* latslotfile = NULL;
_Atomic uint16_t assoc_rnti = 0;
static pid_controller_t pid_ctrl;

// UDP socket globals
static int udp_sockfd = -1;
static struct sockaddr_in udp_addr;

// Config (adjustable)
static bool enoughtime = false; 
double latency_ref_ms = 5.0;           // target latency (ms)
static double dt_default = 0.001;             // sampling period (s)
static double min_tc_interval_ms = 0;  // 0 -> dt
static const double rate_min_mbps = 10; // 10 Mbps min
static const double rate_max_mbps = 1000.0; // 1 Gbps max
static const double min_rate_delta_mbps = 0.5;
static double current_rate_mbps = 10.0; // starting rate 

// -----------------------------
static void sigint_handler(int sig) {
    printf("Signal %d received! Exiting...\n", sig);
    exit_flag = true;
}


static int RunUDP(void) { // ESTA HECHO CON VARIABLES GLOBALES
    udp_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_sockfd < 0) {
        perror("socket creation failed");
        return -1;
    }

    memset(&udp_addr, 0, sizeof(udp_addr));
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(UDP_PORT);
    if (inet_pton(AF_INET, IP_ADDR, &udp_addr.sin_addr) != 1) {
        perror("inet_pton failed");
        close(udp_sockfd);
        udp_sockfd = -1;
        return -1;
    }

    return 0;
}

static double lat_slot = 0.0;
static int lat_count = 0;
static double avg_lat_slot = 0.0;
static int64_t init;

static void sm_cb_new(sm_ag_if_rd_t const* rd, global_e2_node_id_t const* e2_node) {
    if (!rd || !e2_node) return;
    assert(rd->type == INDICATION_MSG_AGENT_IF_ANS_V0);
    assert(rd->ind.type == NEW_STATS_V0);

    
    int64_t now_us = time_now_us();
    int64_t latency_us = now_us - rd->ind.new_sm.msg.tstamp;
    double latency_ms = (double)(latency_us) / 1000.0;
    reg_time_btw_cb = (double)(now_us - last_time_cb_us) / 1000.0;
    last_time_cb_us = now_us;

    if (prev_time == rd->ind.new_sm.msg.tstamp) // duplicate / old heatmap
        return;
    prev_time = rd->ind.new_sm.msg.tstamp;
    // if (latency_ms > reg_time_btw_cb) latency_ms = latency_ms - reg_time_btw_cb/2;
    // printf("Time btw calls: %.2f ms \n", reg_time_btw_cb);

    lat_slot += latency_ms;
    lat_count+=1;
    fprintf(latencyfile, "%lld\t%f\n", (long long) now_us, latency_ms);
    fflush(latencyfile);
    printf("[INFO] time: %.1f | New heatmap arrived, latency: %.2f ms\n", (now_us - init)/1e6,latency_ms);

    // calculate dt in seconds
    double dt = (last_time_us > 0) ? (now_us - last_time_us) / 1e6 : dt_default;
    if (dt <= 0.0) dt = dt_default;
    last_time_us = now_us;

    // Comment
    // double pid_output = pid_compute(&pid_ctrl, latency_ref_ms, latency_ms, dt);
    // //We subtract pid_output because higher latency -> reduce rate
    
    
    // printf("[PID] latency_ref: %f ms | latency_meas: %.2f ms | pid_out: %.6f | new_rate_est: %.2f\n",
    //        latency_ref_ms, latency_ms, pid_output, current_rate_mbps);
    

    

    if (min_tc_interval_ms == 0) min_tc_interval_ms = dt*1000; // meaning when there is a new hetmap
   

    // printf("[time] diff: %f %f\n",  (double)(now_us - last_tc_change_us) / 1000.0, min_tc_interval_ms);
 
    

    if (((double) (now_us - last_tc_change_us)/1000.0) > min_tc_interval_ms) {

        avg_lat_slot = lat_slot/lat_count;
        // printf("[PID] lat_slot: %.2f | lat_count: %i | avg: %.2f \n", lat_slot, lat_count, avg_lat_slot);
        double pid_output = pid_compute(&pid_ctrl, latency_ref_ms, avg_lat_slot, min_tc_interval_ms/1000);
        current_rate_mbps -= pid_output;
        printf("[PID] lat_slot: %.2f | lat_count: %i | avg: %.2f | pid_out: %.3f \n", lat_slot, lat_count, avg_lat_slot, pid_output);
        fprintf(latslotfile, "%lld\t%f\n", (long long) now_us, avg_lat_slot);

        // limiting rate
        if (current_rate_mbps < rate_min_mbps) current_rate_mbps = rate_min_mbps;
        if (current_rate_mbps > rate_max_mbps) current_rate_mbps = rate_max_mbps;
        printf("[PID] current_tc_interval: %2.f ms | new_rate_est: %.2f Mbps\n", (now_us - last_tc_change_us)/1000.0,current_rate_mbps);
         last_tc_change_us = now_us;
        if (udp_sockfd >= 0) {
            char* buf = malloc(sizeof(current_rate_mbps));
            if (buf) {
                memcpy(buf, &current_rate_mbps, sizeof(current_rate_mbps));
                ssize_t sent = sendto(udp_sockfd, buf, sizeof(current_rate_mbps), 0,
                                    (const struct sockaddr *)&udp_addr, sizeof(udp_addr));
                if (sent < 0) {
                    fprintf(stderr, "[UDP] sendto failed!!!!!: %s\n", strerror(errno));
                } else {
                    printf("[UDP] Sent %zd bytes to %s:%d (rate=%.2f Mbps)\n",
                        sent, IP_ADDR, UDP_PORT, current_rate_mbps);
                }
                free(buf);
                fprintf(ratefile, "%lld\t%f\n", (long long) now_us, current_rate_mbps);
                fflush(ratefile);
            }
        }
        lat_count = 0;
        lat_slot = 0;
    }
}

int main(int argc, char *argv[]) {
    
    int kp = 1;
    if (argc >= 4) latency_ref_ms = atof(argv[3]);
    if (argc >= 5) kp = atof(argv[4]);
    if (argc >= 6) min_tc_interval_ms =(atof(argv[5]));

    printf("[CONFIG-PID] latency_ref_ms=%.3f ms | Kp=%i  |  time_inter_tc_calls=%f ms\n",
           latency_ref_ms, kp, min_tc_interval_ms);

    latencyfile = fopen("./pid_results/xappsensectrl_dels.txt", "w");
    if (!latencyfile) {
        perror("fopen latency file");
        return -1;
    }
    ratefile = fopen("./pid_results/xappsensectrl_chann.txt", "w");
    if (!ratefile) {
        perror("fopen rate file");
        fclose(latencyfile);
        return -1;
    }
    latslotfile = fopen("./pid_results/xappsensectrl_slot_lat.txt", "w");
    if (!latencyfile){
        perror("fopen slot del file");
        fclose(latencyfile);
        return -1;
    }

    // initialize UDP notifier
    if (RunUDP() != 0) {
        fprintf(stderr, "Warning: UDP notifier not available\n");
        // continue without UDP
    }

    fr_args_t args = init_fr_args(argc, argv);
    defer({ free_fr_args(&args); });

    pid_init(&pid_ctrl, kp, 0.02, 0.1);

    // pid_init(&pid_ctrl, kp, 0, 0);
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
    init = time_now_us();
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

    for (int i = 0; i < (int)nodes.len; ++i)
        rm_report_sm_xapp_api(sense_handle[i].u.handle);
    free(sense_handle);

    if (latencyfile) fclose(latencyfile);
    if (ratefile) fclose(ratefile);

    if (udp_sockfd >= 0) close(udp_sockfd);

    while(try_stop_xapp_api() == false)
        usleep(1000);

    printf("Test xApp run SUCCESSFULLY\n");
    return 0;
}
