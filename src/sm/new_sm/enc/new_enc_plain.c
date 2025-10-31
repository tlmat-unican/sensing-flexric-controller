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



#include "new_enc_plain.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

byte_array_t new_enc_event_trigger_plain(new_event_trigger_t const* event_trigger)
{
  assert(event_trigger != NULL);
  byte_array_t  ba = {0};
 
  ba.len = sizeof(event_trigger->ms);
  ba.buf = malloc(ba.len);
  assert(ba.buf != NULL && "Memory exhausted");

  memcpy(ba.buf, &event_trigger->ms, ba.len);

  return ba;
}

byte_array_t new_enc_action_def_plain(new_action_def_t const* action_def)
{
  assert(0!=0 && "Not implemented");

  assert(action_def != NULL);
  byte_array_t  ba = {0};
  return ba;
}

byte_array_t new_enc_ind_hdr_plain(new_ind_hdr_t const* ind_hdr)
{
  assert(ind_hdr != NULL);

  byte_array_t ba = {0};

  ba.len = sizeof(new_ind_hdr_t);

  ba.buf = calloc(ba.len, sizeof(uint8_t));
  assert(ba.buf != NULL && "memory exhausted");
  memcpy(ba.buf, ind_hdr, ba.len);

  return ba;
}

byte_array_t new_enc_ind_msg_plain(new_ind_msg_t const* ind_msg)
{
    assert(ind_msg != NULL);

    byte_array_t ba = {0};

    size_t const sz = sizeof(ind_msg->len) + 
                      sizeof(ind_msg->tstamp)+
                      ind_msg->len;

    ba.buf = malloc(sz); 
    assert(ba.buf != NULL && "Memory exhausted");

    // Copy len
    memcpy(ba.buf, &ind_msg->len, sizeof(ind_msg->len));

    // Copy data
    void* it = ba.buf + sizeof(ind_msg->len);
    memcpy(it, ind_msg->data, ind_msg->len);
    it += ind_msg->len;

    // Copy tstamp
    memcpy(it, &ind_msg->tstamp, sizeof(ind_msg->tstamp));
    it += sizeof(ind_msg->tstamp);

    assert(it == ba.buf + sz && "Mismatch of data layout");

    ba.len = sz;
    return ba;
}

byte_array_t new_enc_call_proc_id_plain(new_call_proc_id_t const* call_proc_id)
{
  assert(0!=0 && "Not implemented");

  assert(call_proc_id != NULL);
  byte_array_t  ba = {0};
  return ba;
}

byte_array_t new_enc_ctrl_hdr_plain(new_ctrl_hdr_t const* ctrl_hdr)
{
    assert(ctrl_hdr != NULL);
    byte_array_t ba = {0};

    // printf(" >> new_enc_ctrl_hdr_plain dummy=%i\n", ctrl_hdr->dummy);

    ba.len = sizeof(new_ctrl_hdr_t);
    ba.buf = calloc(ba.len, 1);  // reservar exactamente ba.len bytes
    assert(ba.buf != NULL && "Memory exhausted");

    memcpy(ba.buf, ctrl_hdr, ba.len);

    return ba;
}



byte_array_t new_enc_ctrl_msg_plain(new_ctrl_msg_t const* ctrl_msg)
{
    assert(ctrl_msg != NULL);

    byte_array_t ba = {0};

    // Tamaño total = num_tlvs (4) + msg_len (4) + payload (msg_len bytes)
    ba.len = sizeof(uint32_t) + sizeof(uint32_t) + ctrl_msg->msg_len;
    //  printf("[DEBUG] Total encoded message length = %u bytes msg_len = %i\n", ba.len, ctrl_msg->msg_len);
    // Reservar memoria
    ba.buf = calloc(ba.len, 1);
    assert(ba.buf != NULL && "Memory exhausted");

    char* ptr = (char*)ba.buf;

    // Copiar num_tlvs
    memcpy(ptr, &ctrl_msg->num_tlvs, sizeof(ctrl_msg->num_tlvs));
    ptr += sizeof(ctrl_msg->num_tlvs);

    // Copiar msg_len
    memcpy(ptr, &ctrl_msg->msg_len, sizeof(ctrl_msg->msg_len));
    ptr += sizeof(ctrl_msg->msg_len);

    // Copiar payload si existe
    if (ctrl_msg->msg_len > 0) {
        memcpy(ptr, ctrl_msg->payload, ctrl_msg->msg_len);
    }
  
    return ba;
}


byte_array_t new_enc_ctrl_out_plain(new_ctrl_out_t const* ctrl) 
{
  assert(ctrl != NULL);
  byte_array_t  ba = {0};
  size_t sz = sizeof(ctrl->ans);
  ba.buf = malloc(sz);
  memcpy(ba.buf, &ctrl->ans, sz);
  assert(ba.buf != NULL && "Memory exhausted");
  ba.len = sz;
  return ba;
}

byte_array_t new_enc_func_def_plain(new_func_def_t const* func)
{
  assert(0!=0 && "Not implemented");

  assert(func != NULL);
  byte_array_t  ba = {0};
  return ba;
}

