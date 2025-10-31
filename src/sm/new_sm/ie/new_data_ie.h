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

#ifndef NEW_DATA_INFORMATION_ELEMENTS_H
#define NEW_DATA_INFORMATION_ELEMENTS_H

#ifdef __cplusplus
extern "C" {
#endif

/*
 * 9 Information Elements (IE) , RIC Event Trigger Definition, RIC Action Definition, RIC Indication Header, RIC Indication Message, RIC Call Process ID, RIC Control Header, RIC Control Message, RIC Control Outcome and RAN Function Definition defined by ORAN-WG3.E2SM-v01.00.00 at Section 5
 */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// #define MAX_MSG_SIZE 256  // Puedes ajustar este tamaño máximo

//////////////////////////////////////
// RIC Event Trigger Definition
/////////////////////////////////////

typedef struct {
  uint32_t ms;
} new_event_trigger_t;

void free_new_event_trigger(new_event_trigger_t* src); 

new_event_trigger_t cp_new_event_trigger( new_event_trigger_t* src);

bool eq_new_event_trigger(new_event_trigger_t* m0, new_event_trigger_t* m1);



//////////////////////////////////////
// RIC Action Definition 
/////////////////////////////////////

typedef struct {
  uint32_t dummy;  
} new_action_def_t;

void free_new_action_def(new_action_def_t* src); 

new_action_def_t cp_new_action_def(new_action_def_t* src);

bool eq_new_action_def(new_event_trigger_t* m0,  new_event_trigger_t* m1);



//////////////////////////////////////
// RIC Indication Header 
/////////////////////////////////////

typedef struct{
  uint32_t dummy;  
} new_ind_hdr_t;

void free_new_ind_hdr(new_ind_hdr_t* src); 

new_ind_hdr_t cp_new_ind_hdr(new_ind_hdr_t const* src);

bool eq_new_ind_hdr(new_ind_hdr_t* m0, new_ind_hdr_t* m1);


//////////////////////////////////////
// RIC Indication Message 
/////////////////////////////////////

// Data could be defined here !!

typedef struct {
  float r;
  float theta; // with sign 
  float strength;
} pixel_t;


typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_DOUBLE
} ElementType;




typedef struct {
  
    float theta_init;
    float theta_step;
    float theta_end;


    float r_init;
    float r_step;
    float r_end;
    
    ElementType precision;

    char sensinginfo[];
} sensing_data_t;



typedef struct {
  sensing_data_t* data; // Rx from matlab
  uint16_t len; 
  int64_t tstamp;
} new_ind_msg_t;

void free_new_ind_msg(new_ind_msg_t* src); 

new_ind_msg_t cp_new_ind_msg(new_ind_msg_t const* src);

bool eq_new_ind_msg(new_ind_msg_t* m0, new_ind_msg_t* m1);


//////////////////////////////////////
// RIC Call Process ID 
/////////////////////////////////////

typedef struct {
  int dummy;
} new_call_proc_id_t;

void free_new_call_proc_id( new_call_proc_id_t* src); 

new_call_proc_id_t cp_new_call_proc_id( new_call_proc_id_t* src);

bool eq_new_call_proc_id(new_call_proc_id_t* m0, new_call_proc_id_t* m1);



//////////////////////////////////////
// RIC Control Header 
/////////////////////////////////////

typedef struct {
  int sense_id_device; // 0 - Alldevices

} new_ctrl_hdr_t;

void free_new_ctrl_hdr( new_ctrl_hdr_t* src); 

new_ctrl_hdr_t cp_new_ctrl_hdr(new_ctrl_hdr_t* src);

bool eq_new_ctrl_hdr(new_ctrl_hdr_t* m0, new_ctrl_hdr_t* m1);



//////////////////////////////////////
// RIC Control Message 
/////////////////////////////////////
typedef enum {
  SENSE_CTRL_SELECT_ANGLE,
  SENSE_CTRL_SELECT_RADIO,
  SENSE_CTRL_SELECT_PERIODICITY,
  SENSE_CTRL_SELECT_RESOLUTION,
} e2_sense_ctrl_hdr_t;

typedef struct {
    float theta_init;
    float theta_step;
    float theta_end;
} ctrl_select_angle_t;

typedef struct {
    float period;
} ctrl_select_period_t;

typedef struct {
    ElementType resolution;
} ctrl_select_resolution_t;

typedef struct 
{
  float r_init;
  float r_step;
  float r_end;

}ctrl_select_radio_t;


typedef struct {
    uint8_t type;     // tipo de control
    uint8_t len;      // longitud de value en bytes
    char value[];  // datos del submensaje
} ctrl_tlv_t;

typedef struct {
    uint32_t num_tlvs;  // cuántos submensajes contiene
    uint32_t msg_len;
    ctrl_tlv_t *payload;  // concatenación de TLVs
} new_ctrl_msg_t;



void free_new_ctrl_msg( new_ctrl_msg_t* src); 

new_ctrl_msg_t cp_new_ctrl_msg(new_ctrl_msg_t* src);

bool eq_new_ctrl_msg(new_ctrl_msg_t* m0, new_ctrl_msg_t* m1);



//////////////////////////////////////
// RIC Control Outcome 
/////////////////////////////////////

typedef enum{
  NEW_CTRL_OUT_OK,
  NEW_CTRL_OUT_END
} new_ctrl_out_e;


typedef struct {
  new_ctrl_out_e ans;
} new_ctrl_out_t;

void free_new_ctrl_out(new_ctrl_out_t* src); 

new_ctrl_out_t cp_new_ctrl_out(new_ctrl_out_t* src);

bool eq_new_ctrl_out(new_ctrl_out_t* m0, new_ctrl_out_t* m1);


//////////////////////////////////////
// RAN Function Definition 
/////////////////////////////////////

typedef struct {
  uint8_t* buf;
  size_t len;
} new_func_def_t;

void free_new_func_def( new_func_def_t* src); 

new_func_def_t cp_new_func_def(new_func_def_t const* src);

bool eq_new_func_def(new_func_def_t const* m0, new_func_def_t const* m1);

/////////////////////////////////////////////////
//////////////////////////////////////////////////
/////////////////////////////////////////////////


/*
 * O-RAN defined 5 Procedures: RIC Subscription, RIC Indication, RIC Control, E2 Setup and RIC Service Update 
 * */


///////////////
/// RIC Subscription
///////////////

typedef struct{
  new_event_trigger_t et; 
  new_action_def_t* ad;
} new_sub_data_t;

///////////////
// RIC Indication
///////////////

typedef struct{
  new_ind_hdr_t hdr;
  new_ind_msg_t msg;
  new_call_proc_id_t* proc_id;
} new_ind_data_t;

void free_new_ind_data(new_ind_data_t* ind);

new_ind_data_t cp_new_ind_data(new_ind_data_t const* src);

///////////////
// RIC Control
///////////////

typedef struct{
  new_ctrl_hdr_t hdr;
  new_ctrl_msg_t msg;
} new_ctrl_req_data_t;

typedef struct{
  new_ctrl_out_t* out;
} new_ctrl_out_data_t;

///////////////
// E2 Setup
///////////////

typedef struct{
  new_func_def_t func_def;
} new_e2_setup_data_t;

///////////////
// RIC Service Update
///////////////

typedef struct{
  new_func_def_t func_def;
} new_ric_service_update_t;


#ifdef __cplusplus
}
#endif

#endif

