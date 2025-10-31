#include <stdatomic.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "xApp/e42_xapp_api.h"
#include "util/alg_ds/alg/defer.h"
#include "util/time_now_us.h"
#include "sm/new_sm/new_sm_id.h"

int64_t prev_time = 0;
static bool exit_flag = false;
static void sigint_handler(int sig)
{
  printf("signal %d received !\n", sig);
  exit_flag = true;
}

_Atomic uint16_t assoc_rnti = 0;
static FILE* latencyfile = NULL;

static sm_ag_if_wr_t fill_dummy_sense_sm_ctrl_req(float theta_init_s, float theta_step_s, float theta_end_s,
                                                  float radio_init_s, float radio_step_s, float radio_end_s,
                                                  float period_s, float resolution_s)
{
    sm_ag_if_wr_t wr = {0};
    wr.type = CONTROL_SM_AG_IF_WR;
    wr.ctrl.type = NEW_CTRL_REQ_V0;
    new_ctrl_msg_t msg = {0};

    int num_tlvs = 0;
    if (!isnan(theta_init_s) && !isnan(theta_step_s) && !isnan(theta_end_s)) num_tlvs++;
    if (!isnan(period_s)) num_tlvs++;
    if (!isnan(resolution_s)) num_tlvs++;
    if (!isnan(radio_init_s) && !isnan(radio_step_s) && !isnan(radio_end_s)) num_tlvs++;

    msg.num_tlvs = num_tlvs;

    uint32_t total_size = 0;
    if (!isnan(theta_init_s) && !isnan(theta_step_s) && !isnan(theta_end_s)) total_size += sizeof(ctrl_tlv_t) + sizeof(ctrl_select_angle_t);
    if (!isnan(period_s)) total_size += sizeof(ctrl_tlv_t) + sizeof(ctrl_select_period_t);
    if (!isnan(resolution_s)) total_size += sizeof(ctrl_tlv_t) + sizeof(ctrl_select_resolution_t);
    if (!isnan(radio_init_s) && !isnan(radio_step_s) && !isnan(radio_end_s)) total_size += sizeof(ctrl_tlv_t) + sizeof(ctrl_select_radio_t);

    if (total_size > 0) {
        msg.payload = (ctrl_tlv_t*)malloc(total_size);
        if (msg.payload == NULL) {
            msg.msg_len = 0;
            wr.ctrl.new_ctrl.msg = msg;
            return wr;
        }
    } else {
        msg.payload = NULL;
    }

    msg.msg_len = total_size;
    char *ptr = (char*)msg.payload;

    if (!isnan(theta_init_s) && !isnan(theta_step_s) && !isnan(theta_end_s)) {
        ctrl_tlv_t *tlv = (ctrl_tlv_t*)ptr;
        tlv->type = SENSE_CTRL_SELECT_ANGLE;
        tlv->len = sizeof(ctrl_select_angle_t);
        ctrl_select_angle_t angle = {theta_init_s, theta_step_s, theta_end_s};
        memcpy(tlv->value, &angle, sizeof(angle));
        ptr += sizeof(ctrl_tlv_t) + sizeof(ctrl_select_angle_t);
    }

    if (!isnan(period_s)) {
        ctrl_tlv_t *tlv = (ctrl_tlv_t*)ptr;
        tlv->type = SENSE_CTRL_SELECT_PERIODICITY;
        tlv->len = sizeof(ctrl_select_period_t);
        ctrl_select_period_t per = {period_s};
        memcpy(tlv->value, &per, sizeof(per));
        ptr += sizeof(ctrl_tlv_t) + sizeof(ctrl_select_period_t);
    }

    if (!isnan(resolution_s)) {
        ctrl_tlv_t *tlv = (ctrl_tlv_t*)ptr;
        tlv->type = SENSE_CTRL_SELECT_RESOLUTION;
        tlv->len = sizeof(ctrl_select_resolution_t);
        ctrl_select_resolution_t res = {resolution_s};
        memcpy(tlv->value, &res, sizeof(res));
        ptr += sizeof(ctrl_tlv_t) + sizeof(ctrl_select_resolution_t);
    }

    if (!isnan(radio_init_s) && !isnan(radio_step_s) && !isnan(radio_end_s)) {
        ctrl_tlv_t *tlv = (ctrl_tlv_t*)ptr;
        tlv->type = SENSE_CTRL_SELECT_RADIO;
        tlv->len = sizeof(ctrl_select_radio_t);
        ctrl_select_radio_t radio = {radio_init_s, radio_step_s, radio_end_s};
        memcpy(tlv->value, &radio, sizeof(radio));
        ptr += sizeof(ctrl_tlv_t) + sizeof(ctrl_select_radio_t);
    }

    wr.ctrl.new_ctrl.msg = msg;
    return wr;
}

sm_ag_if_wr_t create_ctrl_msg(int idx)
{
    switch(idx) {
        case 0: return fill_dummy_sense_sm_ctrl_req(NAN,NAN,NAN,NAN,NAN,NAN,NAN,2);
        case 1: return fill_dummy_sense_sm_ctrl_req(NAN,NAN,NAN,NAN,NAN,NAN,NAN,1);
        case 2: return fill_dummy_sense_sm_ctrl_req(-10,3,20,0,1,2,NAN,1);
        case 3: return fill_dummy_sense_sm_ctrl_req(-10,3,20,0,1,2,NAN,2);
        case 4: return fill_dummy_sense_sm_ctrl_req(-45,63,45,0,128,8.5,NAN,2);
        default: return fill_dummy_sense_sm_ctrl_req(0,0,0,0,0,0,0,0);
    }
}

void PrintRawData(unsigned char *buff, int len)
{
  fprintf(stdout, "\n=== Printing raw buffer of %d bytes =======\n", len);
  for (int i = 0; i < len; i++) {
    printf("%02x ", buff[i]);
    if ((i + 1) % 8 == 0) printf("\n");
  }
  printf("\n===========================================\n");
}

void PrintSensingMatrix(const sensing_data_t *sdata)
{
    if (!sdata) return;
    size_t n_theta = (size_t)sdata->theta_step;
    size_t n_r     = (size_t)sdata->r_step;
    fprintf(stdout, ">> n_r %zu n_theta %zu\n", n_r, n_theta);

    if (sdata->precision == TYPE_INT) {
        int32_t *arr = (int32_t*)sdata->sensinginfo;
        for (size_t i = 0; i < n_r; i++) {
            for (size_t j = 0; j < n_theta; j++) printf("%d ", arr[i * n_theta + j]);
            printf("\n");
        }
    } else if (sdata->precision == TYPE_FLOAT) {
        float *arr = (float*)sdata->sensinginfo;
        for (size_t i = 0; i < n_r; i++) {
            for (size_t j = 0; j < n_theta; j++) printf("%f ", arr[i * n_theta + j]);
            printf("\n");
        }
    } else if (sdata->precision == TYPE_DOUBLE) {
        double *arr = (double*)sdata->sensinginfo;
        for (size_t i = 0; i < n_r; i++) {
            for (size_t j = 0; j < n_theta; j++) printf("%lf ", arr[i * n_theta + j]);
            printf("\n");
        }
    }
}

static void sm_cb_new(sm_ag_if_rd_t const* rd, global_e2_node_id_t const* e2_node)
{
  assert(rd != NULL);
  assert(rd->type == INDICATION_MSG_AGENT_IF_ANS_V0);
  assert(rd->ind.type == NEW_STATS_V0);

  int64_t now = time_now_us();
  if (prev_time != rd->ind.new_sm.msg.tstamp) {
      printf("[xApp]: >> Heatmap update - latency = %li ms, len %i, E2-node type %d, ID %d\n",
         (now - rd->ind.new_sm.msg.tstamp)/1000, rd->ind.new_sm.msg.len, e2_node->type, e2_node->nb_id.nb_id);
      fprintf(latencyfile, "%lld\t%lld\n", now, now - rd->ind.new_sm.msg.tstamp);
      prev_time = rd->ind.new_sm.msg.tstamp;
  }
}

int main(int argc, char *argv[])
{

  latencyfile = fopen("xappsensectr.txt", "w");
  fr_args_t args = init_fr_args(argc, argv);
  defer({ free_fr_args(&args); });

  init_xapp_api(&args);
  signal(SIGINT, sigint_handler);
  signal(SIGTERM, sigint_handler);
  sleep(1);

  e2_node_arr_xapp_t nodes = e2_nodes_xapp_api();
  defer({ free_e2_node_arr_xapp(&nodes); });

  assert(nodes.len > 0);
  printf("Connected E2 nodes len = %d\n", nodes.len);

  sm_ans_xapp_t* sense_handle = NULL;
  if(nodes.len > 0){
    sense_handle = calloc(nodes.len, sizeof(sm_ans_xapp_t));
    assert(sense_handle != NULL);
  }

  for(size_t i = 0; i < nodes.len; ++i) {
    e2_node_connected_xapp_t *n = &nodes.n[i];
    for (size_t j = 0; j < n->len_rf; ++j)
      printf("Registered ran func id = %d \n ", n->rf[j].id);

    sense_handle[i] = report_sm_xapp_api(&nodes.n[i].id, SM_NEW_ID, (void*)args.sub_cust_sm[i].time, sm_cb_new);
    assert(sense_handle[i].success == true);
    sleep(2);
  }

  int ncycles = 0;
  while (ncycles < 2){
    for(size_t ni = 0; ni < nodes.len; ++ni) {
      for (int j = 0; j < 5; ++j) {
        sm_ag_if_wr_t ctrl_msg_add = create_ctrl_msg(j);
        control_sm_xapp_api(&nodes.n[ni].id, SM_NEW_ID, &ctrl_msg_add);
        sleep(10);
        if (ctrl_msg_add.ctrl.new_ctrl.msg.payload) {
          free(ctrl_msg_add.ctrl.new_ctrl.msg.payload);
          ctrl_msg_add.ctrl.new_ctrl.msg.payload = NULL;
        }
      }
    }
  }
  // sleep(1000);
  for(int i = 0; i < (int)nodes.len; ++i)
    rm_report_sm_xapp_api(sense_handle[i].u.handle);

  if(nodes.len > 0){
    free(sense_handle);
  }

  // while(try_stop_xapp_api() == false)
  //   usleep(1000);

  if (latencyfile) fclose(latencyfile);
  printf("Test xApp run SUCCESSFULLY\n");
  return 0;
}
