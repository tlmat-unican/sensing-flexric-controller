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


#include "new_dec_plain.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>


new_event_trigger_t new_dec_event_trigger_plain(size_t len, uint8_t const ev_tr[len])
{
  new_event_trigger_t ev = {0};
  memcpy(&ev.ms, ev_tr, sizeof(ev.ms));
  return ev;
}

new_action_def_t new_dec_action_def_plain(size_t len, uint8_t const action_def[len])
{
  assert(0!=0 && "Not implemented");
  assert(action_def != NULL);
  new_action_def_t act_def;// = {0};
  return act_def;
}

new_ind_hdr_t new_dec_ind_hdr_plain(size_t len, uint8_t const ind_hdr[len])
{
  assert(len == sizeof(new_ind_hdr_t)); 
  new_ind_hdr_t ret;
  memcpy(&ret, ind_hdr, len);
  return ret;
}

new_ind_msg_t new_dec_ind_msg_plain(size_t len, uint8_t const ind_msg[len])
{
  
  assert(len >= sizeof(uint16_t) + sizeof(int64_t)); // Al menos len y tstamp
  new_ind_msg_t ret = {0};

  memcpy(&ret.len, ind_msg, sizeof(ret.len));

  size_t expected_size = sizeof(ret.len) + ret.len + sizeof(ret.tstamp);
  assert(len >= expected_size && "Message size too small for declared number of pixels");
  if(ret.len > 0){
    ret.data = calloc(1, ret.len);
    assert(ret.data != NULL && "memory exhausted");
  }

  // void const* it = ind_msg + sizeof(ret.len);
  // for(uint32_t i = 0; i < ret.len; ++i){
  // memcpy(&ret.data[i], it, sizeof(ret.data[i]) );
  // it += sizeof(ret.data[i]); 
  // }
  
  void const* it = ind_msg + sizeof(ret.len);
  memcpy(ret.data, it,  ret.len);
  it += ret.len;

  memcpy(&ret.tstamp, it, sizeof(ret.tstamp));
  it += sizeof(ret.tstamp);

//  memcpy(&ret.slot, it, sizeof(ret.slot));
//  it += sizeof(ret.slot);
 
  assert(it == &ind_msg[len] && "Mismatch of data layout");

  return ret;
}

new_call_proc_id_t new_dec_call_proc_id_plain(size_t len, uint8_t const call_proc_id[len])
{
  assert(0!=0 && "Not implemented");
  assert(call_proc_id != NULL);
}

new_ctrl_hdr_t new_dec_ctrl_hdr_plain(size_t len, uint8_t const ctrl_hdr[len])
{
  assert(len == sizeof(new_ctrl_hdr_t)); 
  new_ctrl_hdr_t ret;
  memcpy(&ret, ctrl_hdr, len);
  return ret;
}

new_ctrl_msg_t new_dec_ctrl_msg_plain(size_t len, uint8_t const ctrl_msg[len])
{
 assert(ctrl_msg != NULL);
    new_ctrl_msg_t ret = {0};

    if (len < sizeof(uint32_t) + sizeof(uint32_t)) {
    
        return ret;
    }

    const char* ptr = (const char*)ctrl_msg;

    // Leer num_tlvs
    memcpy(&ret.num_tlvs, ptr, sizeof(ret.num_tlvs));
    ptr += sizeof(ret.num_tlvs);

    // Leer msg_len
    memcpy(&ret.msg_len, ptr, sizeof(ret.msg_len));
    ptr += sizeof(ret.msg_len);

    // Leer payload
    if (ret.msg_len > 0) {
        ret.payload = malloc(ret.msg_len);
        if (!ret.payload) {
  
            ret.msg_len = 0;
            return ret;
        }
        memcpy(ret.payload, ptr, ret.msg_len);
    } else {
        ret.payload = NULL;
    }
    return ret;
}

new_ctrl_out_t new_dec_ctrl_out_plain(size_t len, uint8_t const ctrl_out[len]) 
{
  assert(0!=0 && "Not implemented");
  assert(ctrl_out!= NULL);
}

new_func_def_t new_dec_func_def_plain(size_t len, uint8_t const func_def[len])
{
  assert(0!=0 && "Not implemented");
  assert(func_def != NULL);
}

