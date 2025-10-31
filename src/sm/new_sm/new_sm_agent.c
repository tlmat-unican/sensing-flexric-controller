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


#include "new_sm_agent.h"
#include "new_sm_id.h"
#include "enc/new_enc_generic.h"
#include "dec/new_dec_generic.h"
#include "../../util/alg_ds/alg/defer.h"


#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct{

  sm_agent_t base;

#ifdef ASN
  new_enc_asn_t enc;
#elif FLATBUFFERS 
  new_enc_fb_t enc;
#elif PLAIN
  new_enc_plain_t enc;
#else
  static_assert(false, "No encryption type selected");
#endif

} sm_new_agent_t;


// Function pointers provided by the RAN for the 
// 5 procedures, 
// subscription, indication, control, 
// E2 Setup and RIC Service Update. 
//
static
sm_ag_if_ans_subs_t on_subscription_new_sm_ag(sm_agent_t const* sm_agent, const sm_subs_data_t* data)
{
  assert(sm_agent != NULL);
  assert(data != NULL);

  sm_new_agent_t* sm = (sm_new_agent_t*)sm_agent;
 
  new_event_trigger_t ev = new_dec_event_trigger(&sm->enc, data->len_et, data->event_trigger);

  sm_ag_if_ans_subs_t ans = {.type = PERIODIC_SUBSCRIPTION_FLRC};
  ans.per.t.ms = ev.ms;
  return ans;
}

static
exp_ind_data_t on_indication_new_sm_ag(sm_agent_t const* sm_agent,void* act_def)
{
//  printf("on_indication NEW called \n");
  assert(sm_agent != NULL);
  assert(act_def == NULL && "Action Definition data not needed for this SM");
  sm_new_agent_t* sm = (sm_new_agent_t*)sm_agent;

  exp_ind_data_t ret = {.has_value = true};

  // Fill Indication Header
  new_ind_hdr_t hdr = {.dummy = 0 };
  byte_array_t ba_hdr = new_enc_ind_hdr(&sm->enc, &hdr );
  ret.data.ind_hdr = ba_hdr.buf;
  ret.data.len_hdr = ba_hdr.len;

  // Fill Indication Message 
  //sm_ag_if_rd_t rd_if = {.type = INDICATION_MSG_AGENT_IF_ANS_V0};
  //rd_if.ind.type = NEW_STATS_V0;

  new_ind_data_t new = {0};
 // Liberate the memory if previously allocated by the RAN. It sucks
  defer({ free_new_ind_hdr(&new.hdr) ;});
  defer({ free_new_ind_msg(&new.msg) ;});
  defer({ free_new_call_proc_id(new.proc_id);});

  if(sm->base.io.read_ind(&new) == false)
    return (exp_ind_data_t){.has_value = false};

  byte_array_t ba = new_enc_ind_msg(&sm->enc, &new.msg);
  ret.data.ind_msg = ba.buf;
  ret.data.len_msg = ba.len;

  // Fill Call Process ID
  ret.data.call_process_id = NULL;
  ret.data.len_cpid = 0;

  return ret;
}

static
 sm_ctrl_out_data_t on_control_new_sm_ag(sm_agent_t const* sm_agent, sm_ctrl_req_data_t const* data)
{
    assert(sm_agent != NULL);
    assert(data != NULL);
    printf("on_control_new_sm_ag\n");

    sm_new_agent_t* sm = (sm_new_agent_t*) sm_agent;

    // Decodificar el control request que llega
    new_ctrl_req_data_t ctrl = {0};

    printf("[NEW_SM]: Header length = %zu\n", data->len_hdr);
    // Decodifica header y libera después automáticamente
    ctrl.hdr = new_dec_ctrl_hdr(&sm->enc, data->len_hdr, data->ctrl_hdr);
    // defer({ free_new_ctrl_hdr(&ctrl.hdr); });
    printf("[NEW_SM]: Data length = %zu\n", data->len_msg);
    // Decodifica el mensaje
    ctrl.msg = new_dec_ctrl_msg(&sm->enc, data->len_msg, data->ctrl_msg);
    // defer({ free_new_ctrl_msg(&ctrl.msg); });
    // printf("[NEW_SM]: Ctrl msg = %zu\n", ctrl.msg.action);
    // printf("Decoded action: %u\n", ctrl.msg.action);
    // Llamar a la función write_ctrl del SM
    sm_ag_if_ans_t ans = sm->base.io.write_ctrl(&ctrl);
    // assert(ans.type == CTRL_OUTCOME_SM_AG_IF_ANS_V0);
    // assert(ans.ctrl_out.type == NEW_CTRL_IF_CTRL_ANS_V0);

    // Liberar lo que haya dentro de ans.ctrl_out.new si hace falta
    // defer({ free_new_ctrl_out(&ans.ctrl_out.new); });

    // Codificar la salida de control a byte array
    byte_array_t ba = new_enc_ctrl_out(&sm->enc, &ans.ctrl_out.new);

    sm_ctrl_out_data_t ret = {0};
    ret.len_out = ba.len;
    ret.ctrl_out = ba.buf;

    return ret;
}

static
sm_e2_setup_data_t on_e2_setup_new_sm_ag(sm_agent_t const* sm_agent)
{
  assert(sm_agent != NULL);
  //printf("on_e2_setup called \n");
  sm_new_agent_t* sm = (sm_new_agent_t*)sm_agent;
  (void)sm;

  sm_e2_setup_data_t setup = {.len_rfd =0, .ran_fun_def = NULL  }; 

  // ToDo: in other SMs we should call the RAN to fulfill this data
  // as it represents the capabilities of the RAN Function

  size_t const sz = strnlen(SM_NEW_STR, 256);
  assert(sz < 256 && "Buffer overeflow?");

  setup.len_rfd = sz;
  setup.ran_fun_def = calloc(1, sz);
  assert(setup.ran_fun_def != NULL);

  memcpy(setup.ran_fun_def, SM_NEW_STR , sz);
  
  /*
  // RAN Function
  setup.rf.definition = cp_str_to_ba(SM_NEW_SHORT_NAME);
  setup.rf.id = SM_NEW_ID;
  setup.rf.rev = SM_NEW_REV;

  setup.rf.oid = calloc(1, sizeof(byte_array_t) );
  assert(setup.rf.oid != NULL && "Memory exhausted");

  *setup.rf.oid = cp_str_to_ba(SM_NEW_OID);
*/

  return setup;
}

static
sm_ric_service_update_data_t on_ric_service_update_new_sm_ag(sm_agent_t const* sm_agent )
{
  assert(sm_agent != NULL);
  assert(0!=0 && "Not implemented");


  printf("on_ric_service_update called \n");
  sm_ric_service_update_data_t dst = {0}; 

  return dst;
}

static
void free_new_sm_ag(sm_agent_t* sm_agent)
{
  assert(sm_agent != NULL);
  sm_new_agent_t* sm = (sm_new_agent_t*)sm_agent;
  free(sm);
}



// General SM information

// Definition
static
char const* def_new_sm_ag(void)
{
  return SM_NEW_STR;
}

// ID
static
uint16_t id_new_sm_ag(void)
{
  return SM_NEW_ID; 
}

  // Revision
static
uint16_t rev_new_sm_ag (void)
{
  return SM_NEW_REV;
}

// OID
static
char const* oid_new_sm_ag (void)
{
  return SM_NEW_OID;
}



sm_agent_t* make_new_sm_agent(sm_io_ag_ran_t io)
{
  sm_new_agent_t* sm = calloc(1, sizeof(sm_new_agent_t));
  assert(sm != NULL && "Memory exhausted!!!");

//  *(uint16_t*)(&sm->base.ran_func_id) = SM_NEW_ID; 

  // Read
  sm->base.io.read_ind = io.read_ind_tbl[NEW_STATS_V0];
  sm->base.io.read_setup = io.read_setup_tbl[NEW_AGENT_IF_E2_SETUP_ANS_V0];
 
  //Write
  sm->base.io.write_ctrl = io.write_ctrl_tbl[NEW_CTRL_REQ_V0];
  sm->base.io.write_subs = io.write_subs_tbl[NEW_SUBS_V0];

  sm->base.free_sm = free_new_sm_ag;
  sm->base.free_act_def = NULL; //free_act_def_new_sm_ag;

  // O-RAN E2SM 5 Procedures
  sm->base.proc.on_subscription = on_subscription_new_sm_ag;
  sm->base.proc.on_indication = on_indication_new_sm_ag;
  sm->base.proc.on_control = on_control_new_sm_ag;
  sm->base.proc.on_ric_service_update = on_ric_service_update_new_sm_ag;
  sm->base.proc.on_e2_setup = on_e2_setup_new_sm_ag;
  sm->base.handle = NULL;

  // General SM information
  sm->base.info.def = def_new_sm_ag;
  sm->base.info.id =  id_new_sm_ag;
  sm->base.info.rev = rev_new_sm_ag;
  sm->base.info.oid = oid_new_sm_ag;

//  assert(strlen(SM_NEW_STR) < sizeof( sm->base.ran_func_name) );
//  memcpy(sm->base.ran_func_name, SM_NEW_STR, strlen(SM_NEW_STR)); 

  return &sm->base;
}

/*
uint16_t id_new_sm_agent(sm_agent_t const* sm_agent )
{
  assert(sm_agent != NULL);
  sm_new_agent_t* sm = (sm_new_agent_t*)sm_agent;
  return sm->base.ran_func_id;
}
*/

