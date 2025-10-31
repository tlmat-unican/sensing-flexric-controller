#include "sm_new.h"
#include "../../../test/rnd/fill_rnd_data_new.h"
#include <assert.h>
#include <stdio.h>

#include <assert.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <poll.h>
#include <unistd.h>
#include <time.h>
#include "../../../src/util/time_now_us.h"
#include <arpa/inet.h> 
#include <pthread.h>   
#include <math.h>
#include <errno.h> // For errors

// Inicialización del buffer compartido
new_ind_msg_t latest_new_data = {0};
pthread_mutex_t latest_new_data_mutex = PTHREAD_MUTEX_INITIALIZER;


pthread_mutex_t ctr_req_data_mutex = PTHREAD_MUTEX_INITIALIZER;
unsigned char send_buf[MAX_SEND_BUF];
size_t send_buf_len = 0;
bool flaghasctrldata = false;


void init_new_sm(void)
{
  // No allocation needed
}

void free_new_sm(void)
{
  // No allocation needed
}



bool read_new_sm(void* data)
{
    // assert(data != NULL);
    // //  assert(data->type == MAC_STATS_V0);

    // new_ind_data_t* new = (new_ind_data_t*)data;
    // // printf("fill_new_ind_data\n");
    // fill_new_ind_data(new);
    // return true;

    assert(data != NULL);

    new_ind_data_t* dst = (new_ind_data_t*)data;

    pthread_mutex_lock(&latest_new_data_mutex);

    dst->msg.len = latest_new_data.len;
    dst->msg.tstamp = latest_new_data.tstamp;
    // dst->msg.data = malloc(latest_new_data.len * sizeof(pixel_t));
    dst->msg.data = malloc(latest_new_data.len);
    if (dst->msg.data == NULL) {
        perror("[SM NEW]: malloc in read_new_sm failed");
        pthread_mutex_unlock(&latest_new_data_mutex);
        return false;
    }

    memcpy(dst->msg.data, latest_new_data.data, latest_new_data.len);

    pthread_mutex_unlock(&latest_new_data_mutex);


    return true;
}

void read_new_setup_sm(void* data)
{
  assert(data != NULL);
  //  assert(data->type == MAC_AGENT_IF_E2_SETUP_ANS_V0 );

  assert(0 !=0 && "Not supported");
}





sm_ag_if_ans_t write_ctrl_new_sm(void const* data)
{
    printf("[E2-Agent]: SENSE CONTROL RX - write_ctrl_new_sm called\n");
    assert(data != NULL);

    const new_ctrl_req_data_t* new_req_ctrl = (const new_ctrl_req_data_t*)data;
    const new_ctrl_msg_t* msg = &new_req_ctrl->msg;
    new_ctrl_req_data_t ctr_req_data;
    printf("[TCP SERVER]: Received control message with %u TLVs (len=%u)\n",
           msg->num_tlvs, msg->msg_len);

    pthread_mutex_lock(&ctr_req_data_mutex);

    // Copiar cabecera y msg en ctr_req_data
    ctr_req_data.hdr = new_req_ctrl->hdr;
    ctr_req_data.msg.num_tlvs = msg->num_tlvs;
    ctr_req_data.msg.msg_len  = msg->msg_len;

    // Preparar buffer de envío
    send_buf_len = sizeof(ctr_req_data.hdr) + sizeof(ctr_req_data.msg.num_tlvs) +
                   sizeof(ctr_req_data.msg.msg_len) + msg->msg_len;

    if (send_buf_len > MAX_SEND_BUF) {
        fprintf(stderr, "Send buffer too small!\n");
        pthread_mutex_unlock(&ctr_req_data_mutex);
        sm_ag_if_ans_t ans = {.type = CTRL_OUTCOME_SM_AG_IF_ANS_V0};
        ans.ctrl_out.type = NEW_CTRL_IF_CTRL_ANS_V0;
        return ans;
    }

    unsigned char *ptr = send_buf;

    // Copiar hdr
    memcpy(ptr, &ctr_req_data.hdr, sizeof(ctr_req_data.hdr));
    ptr += sizeof(ctr_req_data.hdr);

    // Copiar num_tlvs
    memcpy(ptr, &ctr_req_data.msg.num_tlvs, sizeof(ctr_req_data.msg.num_tlvs));
    ptr += sizeof(ctr_req_data.msg.num_tlvs);

    // Copiar msg_len
    memcpy(ptr, &ctr_req_data.msg.msg_len, sizeof(ctr_req_data.msg.msg_len));
    ptr += sizeof(ctr_req_data.msg.msg_len);

    // Copiar payload si existe
    if (msg->msg_len > 0 && msg->payload != NULL) {
        memcpy(ptr, msg->payload, msg->msg_len);
    }

    flaghasctrldata = true;

    pthread_mutex_unlock(&ctr_req_data_mutex);

    sm_ag_if_ans_t ans = {.type = CTRL_OUTCOME_SM_AG_IF_ANS_V0};
    ans.ctrl_out.type = NEW_CTRL_IF_CTRL_ANS_V0;
    ans.ctrl_out.new.ans = NEW_CTRL_OUT_OK;

    return ans;
}
