/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */
#include <stdatomic.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>


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

_Atomic
uint16_t assoc_rnti = 0;

static FILE* latencyfile = NULL; 


static sm_ag_if_wr_t fill_dummy_sense_sm_ctrl_req(float theta_init_s, float theta_step_s, float theta_end_s,
                                                  float radio_init_s, float radio_step_s, float radio_end_s,
                                           float period_s, uint8_t resolution_s)
{

    sm_ag_if_wr_t wr = {0};

    wr.type = CONTROL_SM_AG_IF_WR;
    wr.ctrl.type = NEW_CTRL_REQ_V0;
    new_ctrl_msg_t msg = {0};

    msg.num_tlvs = 4;

    // Calculamos tamaño total de payload
    uint32_t total_size = 0;
    total_size += sizeof(ctrl_tlv_t) + sizeof(ctrl_select_angle_t);
    total_size += sizeof(ctrl_tlv_t) + sizeof(ctrl_select_period_t);
    total_size += sizeof(ctrl_tlv_t) + sizeof(ctrl_select_resolution_t);
    total_size += sizeof(ctrl_tlv_t) + sizeof(ctrl_select_radio_t);

    // Asignamos memoria dinámica
    msg.payload = (ctrl_tlv_t *)malloc(total_size);
    if (!msg.payload) {
        perror("malloc failed");
        return wr;
    }

    printf("TotalSize: %i\n", total_size);
    msg.msg_len = total_size;

    char *ptr = (char *)msg.payload;

    // --- TLV Ángulo ---
    ctrl_tlv_t *tlv_angle = (ctrl_tlv_t *)ptr;
    tlv_angle->type = SENSE_CTRL_SELECT_ANGLE;
    tlv_angle->len = sizeof(ctrl_select_angle_t);
    ctrl_select_angle_t angle = {theta_init_s, theta_step_s, theta_end_s};
    memcpy(tlv_angle->value, &angle, sizeof(angle));
    ptr += sizeof(ctrl_tlv_t) + sizeof(ctrl_select_angle_t);

    // --- TLV Periodicidad ---
    ctrl_tlv_t *tlv_period = (ctrl_tlv_t *)ptr;
    tlv_period->type = SENSE_CTRL_SELECT_PERIODICITY;
    tlv_period->len = sizeof(ctrl_select_period_t);
    ctrl_select_period_t period = {period_s};
    memcpy(tlv_period->value, &period, sizeof(period));
    ptr += sizeof(ctrl_tlv_t) + sizeof(ctrl_select_period_t);

    // --- TLV Resolución ---
    ctrl_tlv_t *tlv_res = (ctrl_tlv_t *)ptr;
    tlv_res->type = SENSE_CTRL_SELECT_RESOLUTION;
    tlv_res->len = sizeof(ctrl_select_resolution_t);
    ctrl_select_resolution_t res = {resolution_s}; // integer
    memcpy(tlv_res->value, &res, sizeof(res));
    ptr += sizeof(ctrl_tlv_t) + sizeof(ctrl_select_resolution_t);


    // --- TLV Resolución ---
    ctrl_tlv_t *tlv_radio = (ctrl_tlv_t *)ptr;
    tlv_radio->type = SENSE_CTRL_SELECT_RADIO;
    tlv_radio->len = sizeof(ctrl_select_radio_t);
    ctrl_select_radio_t radio = {radio_init_s, radio_step_s, radio_end_s};
    memcpy(tlv_radio->value, &radio, sizeof(radio));
    ptr += sizeof(ctrl_tlv_t) + sizeof(ctrl_select_radio_t);

    wr.ctrl.new_ctrl.msg = msg;



    printf("total size payload %i\n", total_size);
    PrintRawData((unsigned char*)msg.payload, msg.msg_len);


    return wr;
}



void PrintRawData(unsigned char *buff, int len)
{
  fprintf (stdout, "\n=== Printing raw buffer of %d bytes =======\n", len);
  for (int i = 0; i < len; i++)
  {
    printf("%02x ", buff[i]);
    if ((i + 1) % 8 == 0)
    {
      printf("\n");
    }
  }
  printf("\n===========================================\n");
}


void PrintSensingMatrix(const sensing_data_t *sdata) {
    if (!sdata) return;

    size_t n_theta = (size_t)sdata->theta_step;
    size_t n_r     = (size_t)sdata->r_step;

    fprintf(stdout, ">> n_r %zu n_theta %zu\n", n_r, n_theta);

    if (sdata->precision == TYPE_INT) {
        int32_t *arr = (int32_t*)sdata->sensinginfo;
        for (size_t i = 0; i < n_r; i++) {
            for (size_t j = 0; j < n_theta; j++)
                printf("%d ", arr[i * n_theta + j]);
            printf("\n");
        }
    } else if (sdata->precision == TYPE_FLOAT) {
        float *arr = (float*)sdata->sensinginfo;
        for (size_t i = 0; i < n_r; i++) {
            for (size_t j = 0; j < n_theta; j++)
                printf("%f ", arr[i * n_theta + j]);
            printf("\n");
        }
    } else if (sdata->precision == TYPE_DOUBLE) {
        double *arr = (double*)sdata->sensinginfo;
        for (size_t i = 0; i < n_r; i++) {
            for (size_t j = 0; j < n_theta; j++)
                printf("%lf ", arr[i * n_theta + j]);
            printf("\n");
        }
    }
}




static
void sm_cb_new(sm_ag_if_rd_t const* rd, global_e2_node_id_t const* e2_node)
{
  assert(rd != NULL);
  assert(rd->type ==INDICATION_MSG_AGENT_IF_ANS_V0);

  assert(rd->ind.type == NEW_STATS_V0);

  int64_t now = time_now_us();
  if (prev_time != rd->ind.new_sm.msg.tstamp){
      // printf("%li\t%li\t%li\n", prev_time, rd->ind.new_sm.msg.tstamp, rd->ind.new_sm.msg.tstamp - prev_time);
      printf("[xApp]: >> Heatmap update - latency = %li ms, len %i, E2-node type %d, ID %d\n",
         (now - rd->ind.new_sm.msg.tstamp)/1000, rd->ind.new_sm.msg.len , e2_node->type, e2_node->nb_id.nb_id);
  
      fprintf(latencyfile, "%li\t%li\n", now, now - rd->ind.new_sm.msg.tstamp); 
      prev_time = rd->ind.new_sm.msg.tstamp;
  } 

  // if(rd->ind.new_sm.msg.len != 0)
    // printf("PrintSensingMatrix\n");
    // PrintRawData(rd->ind.new_sm.msg.data, rd->ind.new_sm.msg.len);
    // PrintSensingMatrix((sensing_data_t*)rd->ind.new_sm.msg.data);
    // PrintRawData(rd->ind.new_sm.msg.data, rd->ind.new_sm.msg.len);

}






int main(int argc, char *argv[])
{

  const char* filename = "xappsensectr.txt" ;
  latencyfile = fopen(filename, "a");
  fr_args_t args = init_fr_args(argc, argv);
  defer({ free_fr_args(&args); }); // como en go - libera

  //Init the xApp
  init_xapp_api(&args);
  signal(SIGINT, sigint_handler); // we override the signal mask set in init_xapp_api()
  signal(SIGTERM, sigint_handler);
  sleep(1);

  e2_node_arr_xapp_t nodes = e2_nodes_xapp_api();
  defer({ free_e2_node_arr_xapp(&nodes); });

  assert(nodes.len > 0);
  printf("Connected E2 nodes len = %d\n", nodes.len);

  // // SLICE indication
  // const char* inter_t = "5_ms";
  sm_ans_xapp_t* sense_handle = NULL;

  if(nodes.len > 0){
    sense_handle = calloc( nodes.len, sizeof(sm_ans_xapp_t) ); 
    assert(sense_handle  != NULL);
  }

  for(size_t i = 0; i < nodes.len; ++i) {
    e2_node_connected_xapp_t *n = &nodes.n[i];
    for (size_t j = 0; j < n->len_rf; ++j)
      printf("Registered ran func id = %d \n ", n->rf[j].id);

    sense_handle[i] = report_sm_xapp_api(&nodes.n[i].id, SM_NEW_ID, (void*)args.sub_cust_sm[i].time, sm_cb_new);
    assert(sense_handle[i].success == true);
    sleep(2);


    // Control ADD slice
    sm_ag_if_wr_t ctrl_msg_add = fill_dummy_sense_sm_ctrl_req(-30,1,30,0,1,5,1,1);
    
    control_sm_xapp_api(&nodes.n[i].id, SM_NEW_ID, &ctrl_msg_add);
   

    sleep(5);
  }

    sleep(1000);

  // Remove the handle previously returned
  for(int i = 0; i < nodes.len; ++i)
    rm_report_sm_xapp_api(sense_handle[i].u.handle);

  if(nodes.len > 0){
    free(sense_handle);
  }



  //Stop the xApp
  while(try_stop_xapp_api() == false)
    usleep(1000);

  printf("Test xApp run SUCCESSFULLY\n");
}