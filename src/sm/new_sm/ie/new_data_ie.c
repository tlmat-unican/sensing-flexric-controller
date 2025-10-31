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


#include "new_data_ie.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


//////////////////////////////////////
// RIC Event Trigger Definition
/////////////////////////////////////

void free_new_event_trigger(new_event_trigger_t* src)
{
  assert(src != NULL);
  assert(0!=0 && "Not implemented" ); 
}

new_event_trigger_t cp_new_event_trigger( new_event_trigger_t* src)
{
  assert(src != NULL);
  assert(0!=0 && "Not implemented" ); 

  new_event_trigger_t et = {0};
  return et;
}

bool eq_new_event_trigger(new_event_trigger_t* m0, new_event_trigger_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  assert(0!=0 && "Not implemented" ); 

  return true;
}


//////////////////////////////////////
// RIC Action Definition 
/////////////////////////////////////

void free_new_action_def(new_action_def_t* src)
{
  assert(src != NULL);

  assert(0!=0 && "Not implemented" ); 
}

new_action_def_t cp_new_action_def(new_action_def_t* src)
{
  assert(src != NULL);

  assert(0!=0 && "Not implemented" ); 
  new_action_def_t ad = {0};
  return ad;
}

bool eq_new_action_def(new_event_trigger_t* m0,  new_event_trigger_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  assert(0!=0 && "Not implemented" ); 

  return true;
}


//////////////////////////////////////
// RIC Indication Header 
/////////////////////////////////////


void free_new_ind_hdr(new_ind_hdr_t* src)
{
  assert(src != NULL);
  (void)src;
}

new_ind_hdr_t cp_new_ind_hdr(new_ind_hdr_t const* src)
{
  assert(src != NULL);
  new_ind_hdr_t dst = {0}; 
  dst.dummy = src->dummy;
  return dst;
}

bool eq_new_ind_hdr(new_ind_hdr_t* m0, new_ind_hdr_t* m1)
{
  assert(m0 != 0);
  assert(m1 != 0);

  if(m0->dummy != m1->dummy)
    return false;
  return true;
}






//////////////////////////////////////
// RIC Indication Message 
/////////////////////////////////////

void free_new_ind_msg(new_ind_msg_t* src)
{
  assert(src != NULL);

  if(src->len > 0){
    assert(src->data != NULL);
    free(src->data);
  }
}

new_ind_msg_t cp_new_ind_msg(new_ind_msg_t const* src)
{
  assert(src != NULL);
  
  new_ind_msg_t cp = {
    .len = src->len,
    .tstamp = src->tstamp,
    .data = NULL
  };

  if (cp.len > 0) {
    cp.data = malloc(cp.len);
    assert(cp.data != NULL);  // O maneja el error apropiadamente
    memcpy(cp.data, src->data, cp.len);
  }

  return cp;
}

bool eq_new_ind_msg(new_ind_msg_t* m0, new_ind_msg_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  if (m0->tstamp != m1->tstamp)
    return false;

  if (m0->len != m1->len)
    return false;

  if (memcmp(m0->data, m1->data, m0->len) != 0)
    return false;

  return true;
}

//////////////////////////////////////
// RIC Call Process ID 
/////////////////////////////////////

void free_new_call_proc_id(new_call_proc_id_t* src)
{
  // Note that the src could be NULL
  free(src);
}

new_call_proc_id_t cp_new_call_proc_id( new_call_proc_id_t* src)
{
  assert(src != NULL); 
  new_call_proc_id_t dst = {0};

  dst.dummy = src->dummy;

  return dst;
}

bool eq_new_call_proc_id(new_call_proc_id_t* m0, new_call_proc_id_t* m1)
{
  if(m0 == NULL && m1 == NULL)
    return true;
  if(m0 == NULL)
    return false;
  if(m1 == NULL)
    return false;

  if(m0->dummy != m1->dummy)
    return false;

  return true;
}


//////////////////////////////////////
// RIC Control Header 
/////////////////////////////////////

void free_new_ctrl_hdr( new_ctrl_hdr_t* src)
{

  assert(src != NULL);
  (void)src;
}

new_ctrl_hdr_t cp_new_ctrl_hdr(new_ctrl_hdr_t* src)
{
  assert(src != NULL);
  assert(0!=0 && "Not implemented" ); 
  printf("============= cp_new_ctrl_hdr =============\n");
  new_ctrl_hdr_t ret = {.sense_id_device = src->sense_id_device};
  return ret;
}

bool eq_new_ctrl_hdr(new_ctrl_hdr_t* m0, new_ctrl_hdr_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  assert(0!=0 && "Not implemented" ); 

  return true;
}


//////////////////////////////////////
// RIC Control Message 
/////////////////////////////////////


void free_new_ctrl_msg( new_ctrl_msg_t* src)
{
  assert(src != NULL);

  (void)src;
}

new_ctrl_msg_t cp_new_ctrl_msg(new_ctrl_msg_t* src)
{
    assert(src != NULL);

    new_ctrl_msg_t cp = {
        .num_tlvs = src->num_tlvs,
        .msg_len = src->msg_len,
        .payload = NULL
    };

    if (cp.msg_len > 0 && src->payload != NULL) {
        cp.payload = malloc(cp.msg_len);
        if (cp.payload == NULL) {
            // Manejo de error: deja el payload NULL y msg_len en 0
            cp.msg_len = 0;
            cp.num_tlvs = 0;
            // Podrías también abortar, o registrar el error, según tu política
        } else {
            memcpy(cp.payload, src->payload, cp.msg_len);
        }
    }

    return cp;
}

bool eq_new_ctrl_msg(new_ctrl_msg_t* m0, new_ctrl_msg_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  assert(0!=0 && "Not implemented" ); 

  return true;
}


//////////////////////////////////////
// RIC Control Outcome 
/////////////////////////////////////

void free_new_ctrl_out(new_ctrl_out_t* src)
{
  assert(src != NULL);

  (void)src;
}

new_ctrl_out_t cp_new_ctrl_out(new_ctrl_out_t* src)
{
  assert(src != NULL);

  assert(0!=0 && "Not implemented" ); 
  new_ctrl_out_t ret = {0}; 
  return ret;
}

bool eq_new_ctrl_out(new_ctrl_out_t* m0, new_ctrl_out_t* m1)
{
  assert(m0 != NULL);
  assert(m1 != NULL);

  assert(0!=0 && "Not implemented" ); 

  return true;
}


//////////////////////////////////////
// RAN Function Definition 
/////////////////////////////////////

void free_new_func_def( new_func_def_t* src)
{
  assert(src != NULL);
  free(src->buf);
}

new_func_def_t cp_new_func_def(new_func_def_t const* src)
{
  assert(src != NULL);
  new_func_def_t dst = {.len = src->len};
  if(src->len > 0){
    dst.buf = calloc(dst.len, sizeof(int)); 
    assert(dst.buf != NULL && "memory exhausted");
    memcpy(dst.buf, src->buf, dst.len);
  }

  return dst;
}

bool eq_new_func_def(new_func_def_t const* m0, new_func_def_t const* m1)
{
  if(m0 == m1)
    return true;

  if(m0 == NULL || m1 == NULL)
    return false;

  if(m0->len != m1->len)
    return false;

  int rc = memcmp(m0, m1, m0->len);
  return rc == 0;
}


///////////////
// RIC Indication
///////////////

void free_new_ind_data(new_ind_data_t* ind)
{
  assert(ind != NULL);
  
  free_new_ind_hdr(&ind->hdr);
  free_new_ind_msg(&ind->msg);
  free_new_call_proc_id(ind->proc_id); 
}

new_ind_data_t cp_new_ind_data(new_ind_data_t const* src)
{
  assert(src != NULL);
  new_ind_data_t dst = {0};

  dst.hdr = cp_new_ind_hdr(&src->hdr);
  dst.msg = cp_new_ind_msg(&src->msg);

  if(src->proc_id != NULL){
    dst.proc_id = malloc(sizeof(new_call_proc_id_t));
    assert(dst.proc_id != NULL && "Memory exhausted");
    *dst.proc_id = cp_new_call_proc_id(src->proc_id);
  }

  return dst;
}


