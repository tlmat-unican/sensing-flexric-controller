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

#if defined(__clang__) || defined (__GNUC__)
# define ATTRIBUTE_NO_SANITIZE_THREAD  __attribute__((no_sanitize("thread")))
#else
# define ATTRIBUTE_NO_SANITIZE_THREAD
#endif


#include "../../../src/agent/e2_agent_api.h"
#include "../../../src/util/alg_ds/alg/defer.h"

#include "read_setup_ran.h"
#include "sm_mac.h"
#include "sm_rlc.h"
#include "sm_pdcp.h"
#include "sm_gtp.h"
#include "sm_slice.h"
#include "sm_tc.h"
#include "sm_kpm.h"
#include "sm_rc.h"
#include "sm_new.h"
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

#define SERVER_PORT 12345       
#define BUFFER_SIZE 1024     
#define MAX_CLIENTS 10


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


// The most IMPORTANT function of this file that processes the new SM data received from the socket !!!!
// In order to parse the new SM data received from the UDP socket
// We do the inverse process of the one done in the sensing-node sRU
// -> extract from the socket buffer the corresponding fields (parsed in a auxiliar struct defined at the service model)
// -> by means of a mutex, update the global variable `latest_new_data` 
void process_new_sm_data(const char *data, size_t len) {
    size_t offset = 0;
    new_ind_msg_t msg;

    if (len < sizeof(float)) {
        fprintf(stderr, "[SM NEW]: Packet too small (needs at least len field)\n");
        return;
    }

   
    float len_float = 0;
    memcpy(&len_float, data + offset, sizeof(float));
    offset += sizeof(float);

    
    msg.len = (uint16_t)len_float;

    size_t expected_payload_size = (size_t)msg.len;

  
    if (offset + expected_payload_size > len) {
        fprintf(stderr, "[SM NEW]: Payload length mismatch (needed: %zu, got: %zu)\n",
                offset + expected_payload_size, len);
        return;
    }

   
    msg.data = malloc(expected_payload_size);
    if (msg.data == NULL) {
        perror("[SM NEW]: malloc failed");
        return;
    }

  
    memcpy(msg.data, data + offset, expected_payload_size);

    pthread_mutex_lock(&latest_new_data_mutex);

   
    latest_new_data.len = expected_payload_size;
    latest_new_data.tstamp = (int64_t)time_now_us();

    
    if (latest_new_data.data != NULL) {
        free(latest_new_data.data);
    }

    latest_new_data.data = malloc(expected_payload_size);
    if (latest_new_data.data != NULL) {
        memcpy(latest_new_data.data, msg.data, expected_payload_size);
    } else {
        perror("[SM NEW]: malloc failed for latest_new_data.data");
        latest_new_data.len = 0;
    }

    pthread_mutex_unlock(&latest_new_data_mutex);

    printf("[TCP SERVER]: NEW SM data received: %d bytes, timestamp=%ld\n",
           msg.len, latest_new_data.tstamp);

    printf("[SERVER]: N pixels received: %zu\n", latest_new_data.len / sizeof(pixel_t));

 
    
    for (size_t i = 0; i < latest_new_data.len / sizeof(pixel_t); ++i) {
        printf("[UDP SERVER]: Pixel[%zu] = (%f, %f, %f)\n", i,
               latest_new_data.data[i].r,
               latest_new_data.data[i].theta,
               latest_new_data.data[i].strength);
    }
    

    free(msg.data);
}

void *udp_socket_thread(void *arg) {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[10000];
    socklen_t addr_len = sizeof(client_addr);


    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("[UDP SERVER]: Error at socket creation");
        pthread_exit(NULL);
    }

 
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY; // Listening at all interfaces !
    server_addr.sin_port = htons(SERVER_PORT);


    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("[UDP SERVER]: Error at binding socket");
        close(sockfd);
        pthread_exit(NULL);
    }

    printf("[UDP SERVER]: Listening at port %d...\n", SERVER_PORT);


    while (1) {

        uint32_t accLen = 0;
        uint16_t totalLen = 0;
        
        
        ssize_t len = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
        memcpy(&totalLen, buffer, sizeof(totalLen));
        totalLen = totalLen +sizeof(totalLen);
        accLen = len;
        printf("[UDP SERVER] First received %d/%d Bytes\n", accLen, totalLen);
        while (accLen < totalLen){
          ssize_t len = recvfrom(sockfd, buffer+totalLen, totalLen-accLen, 0, (struct sockaddr *)&client_addr, &addr_len);
          accLen = accLen + len;
          if (len < 0) {
              perror("[UDP SERVER]: Error at recvfrom");
              break;
          }
          printf("[UDP SERVER]: %ld Bytes recived (%d/%d) from %s:%d\n", len, accLen, totalLen, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
          
        }
        printf("[UDP SERVER]: Total %d/%d Bytes recived from %s:%d\n", accLen, totalLen, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        //process_new_sm_data(buffer, len);
    }
    close(sockfd);
    pthread_exit(NULL);
}

void *tcp_server_func(void *sockPtr)
{
  int sock = *(int *)sockPtr;
  char buffer[10000];
  pid_t tid = gettid();

  while (1)
  {
    memset(buffer, 0, sizeof(buffer));
    fprintf(stdout, "[TCP SERVER (%d)] Prepared to receive\n", tid);
    uint32_t accLen = 0;
    float totalLen = 0;
    ssize_t len = recv(sock, buffer, BUFFER_SIZE, 0);
    if (len <= 0)
    {
      fprintf(stderr, "[TCP SERVER (%d)]: Recv <= 0 Bytes: %s", tid, strerror(errno));
      break;
    }
    memcpy(&totalLen, buffer, sizeof(totalLen));
    totalLen = totalLen + sizeof(totalLen);
    accLen = len;
    fprintf(stdout, "[TCP SERVER (%d)] First received %d/%d Bytes\n", tid, accLen, totalLen);
    while (accLen < totalLen)
    {
      ssize_t len = recv(sock, buffer + accLen, totalLen - accLen, 0);
      accLen = accLen + len;
      if (len <= 0)
      {
        fprintf(stderr, "[TCP SERVER (%d)]: Recv <= 0 Bytes: %s", tid, strerror(errno));
        break;
      }
      printf("[TCP SERVER (%d)]: %ld Bytes received (%i/%i)\n", tid, len, accLen, totalLen);
    }
    printf("[TCP SERVER (%d)]: Total %i/%f Bytes recieved\n", tid, accLen, totalLen);
    // uint16_t nPixels = (uint16_t)(totalLen - sizeof(totalLen)) / sizeof(pixel_t);
    // memcpy(buffer, &nPixels, sizeof(nPixels));
    // uint16_t nPixels = (uint16_t)(totalLen - sizeof(totalLen));
    // memcpy(buffer, &nPixels, sizeof(nPixels));
    
    process_new_sm_data(buffer, accLen);
  }
  printf("[TCP SERVER (%d)]: leaving\n", tid);
  close(sock);
  pthread_exit(NULL);
}

void *tcp_server_thread(void *arg)
{
  struct sockaddr_in server_addr;
  int sockfd = socket(PF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
  {
    fprintf(stderr, "[TCP SERVER]: Error at socket creation: %s\n", strerror(errno));
    pthread_exit(NULL);
  }

  int opt = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
  {
    fprintf(stderr, "Error setting socket options: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY; // Listening at all interfaces !
  server_addr.sin_port = htons(SERVER_PORT);

  if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
  {
    fprintf(stderr, "[TCP SERVER]: Error at binding socket: %s\n", strerror(errno));
    close(sockfd);
    pthread_exit(NULL);
  }

  if (listen(sockfd, 1) < 0)
  {
    fprintf(stderr, "[TCP SERVER]: Error at listen: %s\n", strerror(errno));
    close(sockfd);
    pthread_exit(NULL);
  }
  printf("[TCP SERVER]: Listening at port %d...\n", SERVER_PORT);

  int clientCounter = 0;
  pthread_t clientThreads[MAX_CLIENTS];
  while (clientCounter < MAX_CLIENTS)
  {
    struct sockaddr_in client_addr;
    socklen_t addr_size = sizeof(struct sockaddr_storage);
    int clientSockfd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size);
    if (clientSockfd < 0)
    {
      fprintf(stderr, "[TCP SERVER]: Error at accept: %s\n", strerror(errno));
      continue;
    }
    else
    {
      fprintf(stdout, "[TCP SERVER]: Connection accepted from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
      if (pthread_create(&clientThreads[clientCounter], NULL, tcp_server_func, &clientSockfd) != 0)
      {
        fprintf(stderr, "Error creating a thread: %s\n", strerror(errno));
        exit(EXIT_FAILURE);
      }
    }
    ++clientCounter;
  }
  fprintf(stdout, "-- Finish service, connections %d\n", clientCounter);
  for (int i = 0; i < MAX_CLIENTS; i++)
  {
    fprintf(stdout, "waiting for client %d\n", i);
    pthread_join(clientThreads[i], NULL);
  }
  
  fflush(stdout);
  close(sockfd);
  pthread_exit(NULL);
}


static
void init_read_ind_tbl(read_ind_fp (*read_ind_tbl)[SM_AGENT_IF_READ_V0_END])
{

  (*read_ind_tbl)[KPM_STATS_V3_0] = read_kpm_sm ;
  (*read_ind_tbl)[RAN_CTRL_STATS_V1_03] = read_rc_sm;
  (*read_ind_tbl)[NEW_STATS_V0] = read_new_sm;
}

static
void init_read_setup_tbl(read_e2_setup_fp (*read_setup_tbl)[SM_AGENT_IF_E2_SETUP_ANS_V0_END])
{

  (*read_setup_tbl)[KPM_V3_0_AGENT_IF_E2_SETUP_ANS_V0] = read_kpm_setup_sm ;
  (*read_setup_tbl)[RAN_CTRL_V1_3_AGENT_IF_E2_SETUP_ANS_V0] = read_rc_setup_sm;
  (*read_setup_tbl)[NEW_AGENT_IF_E2_SETUP_ANS_V0] = read_new_setup_sm;
}

static
void init_write_ctrl( write_ctrl_fp (*write_ctrl_tbl)[SM_AGENT_IF_WRITE_CTRL_V0_END])
{

  (*write_ctrl_tbl)[RAN_CONTROL_CTRL_V1_03] =  write_ctrl_rc_sm;
  (*write_ctrl_tbl)[NEW_CTRL_REQ_V0] = write_ctrl_new_sm;
}


static
void init_write_subs(write_subs_fp (*write_subs_tbl)[SM_AGENT_IF_WRITE_SUBS_V0_END])
{

  (*write_subs_tbl)[KPM_SUBS_V3_0] = NULL;
  (*write_subs_tbl)[RAN_CTRL_SUBS_V1_03] = write_subs_rc_sm;
  (*write_subs_tbl)[NEW_SUBS_V0] = NULL;
 
}

static
void init_sm(void)
{
  init_kpm_sm(); // This must be here; otherwise, the code will break.
  init_rc_sm();  // This must be here; otherwise, the code will break.
  init_new_sm();
}

static
sm_io_ag_ran_t init_io_ag(void)
{
  sm_io_ag_ran_t io = {0};
  init_read_ind_tbl(&io.read_ind_tbl);
  init_read_setup_tbl(&io.read_setup_tbl);
#if defined(E2AP_V2) || defined(E2AP_V3)
  io.read_setup_ran = read_setup_ran;
#endif
  init_write_ctrl(&io.write_ctrl_tbl);
  init_write_subs(&io.write_subs_tbl);

  init_sm();


  return io;
}

static
void free_io_ag(void)
{
  free_kpm_sm();
}



ATTRIBUTE_NO_SANITIZE_THREAD static
void stop_and_exit()
{
  // Stop the E2 Agent
  stop_agent_api();
  exit(EXIT_SUCCESS);
}

static
pthread_once_t once = PTHREAD_ONCE_INIT;

static
void sig_handler(int sig_num)
{
  printf("\n[E2 AGENT]: Abruptly ending with signal number = %d\n[E2 AGENT]: Please, wait.\n", sig_num);
  // For the impatient, do not break my code
  pthread_once(&once, stop_and_exit);
}

int main(int argc, char *argv[])
{
  // Signal handler
  
  printf("[E2-AGENT]: Starting E2 Agent\n");
  signal(SIGINT, sig_handler);

  // Init the Agent
  // Values defined in the CMakeLists.txt file
  const e2ap_ngran_node_t ran_type = TEST_AGENT_RAN_TYPE;
  const int mcc = TEST_AGENT_MCC;
  const int mnc = TEST_AGENT_MNC;
  const int mnc_digit_len = TEST_AGENT_MNC_DIG_LEN;
  const int nb_id = TEST_AGENT_NB_ID;
  const int cu_du_id = TEST_AGENT_CU_DU_ID;

  sm_io_ag_ran_t io = init_io_ag();

  fr_args_t args = init_fr_args(argc, argv);
  defer({ free_fr_args(&args); });

  if (E2AP_NODE_IS_MONOLITHIC(ran_type))
    printf("[E2-AGENT]: nb_id %d, mcc %d, mnc %d, mnc_digit_len %d, ran_type %s\n", nb_id, mcc, mnc, mnc_digit_len, get_e2ap_ngran_name(ran_type));
  else
    printf("[E2-AGENT]: nb_id %d, mcc %d, mnc %d, mnc_digit_len %d, ran_type %s, cu_du_id %d\n", nb_id, mcc, mnc, mnc_digit_len, get_e2ap_ngran_name(ran_type), cu_du_id);
  
  init_agent_api(mcc, mnc, mnc_digit_len, nb_id, cu_du_id, ran_type, io, &args);

  // pthread_t udp_thread;
  // if (pthread_create(&udp_thread, NULL, udp_socket_thread, NULL) != 0) {
  //     perror("[E2-AGENT]: Error al crear el hilo del socket UDP");
  //     return EXIT_FAILURE;
  // }
  
  pthread_t tcp_thread;
  if (pthread_create(&tcp_thread, NULL, tcp_server_thread, NULL) !=0 ){
      perror("[E2-AGENT]: Error al crear el hilo del socket TCP");
      return EXIT_FAILURE;
  }


  while(1){
    poll(NULL, 0, 1000);
  }

  // Now the E2 Agent is running and listening for incoming messages from the usurp in another thread, thus not blocking the main thread
  // pthread_join(udp_thread, NULL);
  pthread_join(tcp_thread, NULL);
  free_io_ag();

  return EXIT_SUCCESS;
}
