#include <assert.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <poll.h>
#include <pthread.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include "../../src/util/time_now_us.h"
#include <pthread.h>
#include <errno.h> // For errors

#define UDP_PORT 12345
#define SERVER_PORT 12345
#define SERVER_IP "10.10.0.76"
#define BUFFER_SIZE 10000

typedef struct
{
  float r;
  float theta; // with sign
  float strength;
} pixel_t;

void PrintData(char *buff, int nPixels)
{
  pixel_t *aux = (pixel_t *)buff;
  for (int pixelIdx = 0; pixelIdx < nPixels; ++pixelIdx)
  {
    printf("Pixel %d: (%f, %f, %f)\n", pixelIdx, aux[pixelIdx].r, aux[pixelIdx].theta, aux[pixelIdx].strength);
  }
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

int FillInBuffer(int nAngles, int nRanges, char *buff, int maxLen)
{
  float nPixels = nAngles * nRanges;
  uint32_t totalLen = sizeof(nPixels) + nPixels * sizeof(pixel_t);
  assert(totalLen <= maxLen && "Check the lengths");
  float auxLen = nPixels * sizeof(pixel_t);
  printf("Total Length bytes %f\n", auxLen);
  memcpy(buff, &auxLen, sizeof(auxLen));

  pixel_t *aux = (pixel_t *)(buff + sizeof(nPixels));

  for (int pixelIdx = 0; pixelIdx < nPixels; ++pixelIdx)
  {
    aux[pixelIdx].r = (int)(pixelIdx / nRanges);
    aux[pixelIdx].theta = pixelIdx % nRanges;
    aux[pixelIdx].strength = (float)rand() / RAND_MAX;
  }
  return totalLen;
}

void RunTcp()
{

  struct sockaddr_in srvAddr;
  memset(&srvAddr, 0, sizeof(srvAddr));

  int sockfd = socket(PF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
  {
    fprintf(stdout, "Client Connected\n");
    exit(EXIT_FAILURE);
  }

  fprintf(stdout, "Server socket created\n");

  srvAddr.sin_family = AF_INET;
  srvAddr.sin_addr.s_addr = inet_addr(SERVER_IP);
  srvAddr.sin_port = htons(SERVER_PORT);

  socklen_t addrSize = sizeof(srvAddr);
  if (connect(sockfd, (struct sockaddr *)&srvAddr, addrSize) < 0)
  {
    fprintf(stdout, "Client connection error: %s\n", strerror(errno));
    exit(EXIT_FAILURE);
  }

  char buffer[BUFFER_SIZE];
  int nAngles = 15;
  int nRanges = 18;
  while (1)
  {
    int len = FillInBuffer(nAngles, nRanges, buffer, BUFFER_SIZE);
    PrintData(buffer + 4, nAngles * nRanges);
    // PrintRawData (buffer, len);
    send(sockfd, buffer, len, 0);
    sleep(2);
    break; // To send only one message; in order to send it periodically comment the break line
  }

  close(sockfd);
}

int main()
{
  srand(time(NULL));
  RunTcp();
  

  // int sockfd;
  // struct sockaddr_in servaddr;
  // char* buffer;

  // if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
  //     perror("socket creation failed");
  //     exit(EXIT_FAILURE);
  // }

  // memset(&servaddr, 0, sizeof(servaddr));
  // servaddr.sin_family = AF_INET;
  // servaddr.sin_port = htons(UDP_PORT);
  // inet_pton(AF_INET, SERVER_IP, &servaddr.sin_addr);

  // int nAngles = 15;
  // int nRanges= 18;
  // uint16_t nPixels = nAngles*nRanges;
  // uint32_t totalLen = sizeof(nPixels) + nPixels*sizeof(pixel_t);
  // buffer = malloc(totalLen);
  // uint16_t auxLen = nPixels*sizeof(pixel_t);
  // memcpy(buffer, &auxLen, sizeof(auxLen));

  // pixel_t* aux = (pixel_t*) (buffer + sizeof(nPixels));

  // while (1) {
  //   for (int pixelIdx = 0; pixelIdx < nPixels; ++pixelIdx) {
  //     aux[pixelIdx].x = (int)(pixelIdx/nRanges);
  //     aux[pixelIdx].y = pixelIdx%nRanges;
  //     aux[pixelIdx].z = (float)rand()/RAND_MAX;
  //   }

  //   for (int pixelIdx = 0; pixelIdx < nPixels; ++pixelIdx) {
  //     printf("Pixel %d: (%f, %f, %f)\n", pixelIdx, aux[pixelIdx].x, aux[pixelIdx].y, aux[pixelIdx].z);
  //   }
  //   // // Send the message
  //   ssize_t sent = sendto(sockfd, buffer, totalLen, 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
  //   if (sent < 0) {
  //       perror("sendto failed");
  //   } else {
  //       printf("Sent %zd bytes to %s:%d\n", sent, SERVER_IP, UDP_PORT);
  //   }

  //   sleep(5);
  //   // break;
  // }

  // free(buffer);
  // close(sockfd);
  return 0;
}
