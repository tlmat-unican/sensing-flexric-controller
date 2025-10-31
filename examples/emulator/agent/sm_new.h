#ifndef SM_NEW_READ_WRITE_AGENT_H
#define SM_NEW_READ_WRITE_AGENT_H

#include "../../../src/agent/e2_agent_api.h"
#include <pthread.h>

#define MAX_SEND_BUF 4096
extern new_ind_msg_t latest_new_data;
extern pthread_mutex_t latest_new_data_mutex;


extern pthread_mutex_t ctr_req_data_mutex;
extern unsigned char send_buf[MAX_SEND_BUF];
extern size_t send_buf_len;
extern bool flaghasctrldata; 




void init_new_sm(void);

void free_new_sm(void);

bool read_new_sm(void*);

void read_new_setup_sm(void*);

sm_ag_if_ans_t write_ctrl_new_sm(void const*);

#endif
