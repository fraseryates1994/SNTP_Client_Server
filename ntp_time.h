#ifndef _NTP_TIME_H
#define _NTP_TIME_H
#include <stdint.h>
#include <sys/time.h>
#include <time.h>

#define EPOCH 2208988800ull

typedef struct
{
  uint32_t second;
  uint32_t fraction;
} ntp_timestamp; // 64 bit

typedef struct
{

  unsigned li   : 2;       // 2 bit Leap indicator.
  unsigned vn   : 3;       // 3 bitVersion number of the protocol.
  unsigned mode : 3;       // 3 bit Mode. Client will pick mode 3 for client.

  uint8_t stratum;         // 8 bit Stratum level of the local clock.
  uint8_t poll;            // 8 bit Maximum interval between successive messages.
  uint8_t precision;       // 8 bitPrecision of the local clock.

  uint32_t rootDelay;      // 32 bit  Total round trip delay time.
  uint32_t rootDispersion; // 32 bit  Max error aloud from primary clock source.
  char refId[4];          // 32 bit  Reference clock identifier.

  ntp_timestamp refTm;        // 32 bit  Reference time-stamp seconds.


  ntp_timestamp origTm;       // 32 bit  Originate time-stamp seconds.


  ntp_timestamp rxTm;         // 32 bit  Received time-stamp seconds.


  ntp_timestamp txTm;         // 32 bit  Transmit time-stamp seconds.


} ntp_packet;                 // Total: 384 bits or 48 bytes.

ntp_timestamp getCurrentTimestamp();
void convert_unix_time_into_ntp_time(struct timeval *tv, ntp_timestamp *ntp);
void convert_ntp_time_into_unix_time(ntp_timestamp *ntp, struct timeval *tv);
void convertHostToNetwork(ntp_timestamp *ntp);
void convertNetworkToHost(ntp_timestamp *ntp);


#endif
