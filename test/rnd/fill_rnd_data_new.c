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



 #include "fill_rnd_data_new.h"
 #include "../../src/util/time_now_us.h"
 
 #include <assert.h>
 #include <math.h>
 #include <stdlib.h>
 #include <string.h>
 #include <time.h>
 
 
 void fill_new_ind_data(new_ind_data_t* ind){
    



    assert(ind != NULL);

    new_ind_msg_t* ind_msg = &ind->msg;
    ind_msg->len = 256;

    // Asignar memoria para el buffer de datos
    ind_msg->data = (uint8_t*)malloc(ind_msg->len * sizeof(uint8_t));
    assert(ind_msg->data != NULL);  // Asegura que la asignación fue exitosa

    // Rellenar el array con el valor 0x01
    memset(ind_msg->data, 0x01, ind_msg->len);

    ind_msg->tstamp = (int64_t)time_now_us();
  

 }