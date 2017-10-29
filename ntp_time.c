#include "ntp_time.h"
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>

/* referenced from http://waitingkuo.blogspot.co.uk/2012/06/conversion-between-ntp-time-and-unix.html
* Function: convert_ntp_time_into_unix_time
* Parameters: ntp_timestamp and timeval structs
* Returns: void
* Comments: takes in a pointer of an ntp timestamp and converts the timeval to unix time.
*           Converts from 1900 (ntp timestamp) to 1970 (unix timeval) time
*/
void convert_ntp_time_into_unix_time(ntp_timestamp *ntp, struct timeval *tv)
{
    tv->tv_sec = ntp->second - 0x83AA7E80; // the seconds from Jan 1, 1900 to Jan 1, 1970
    tv->tv_usec = (uint32_t)( (double)ntp->fraction * 1.0e6 / (double)(1LL<<32) );
}

/* referenced from http://waitingkuo.blogspot.co.uk/2012/06/conversion-between-ntp-time-and-unix.html
* Function: convert_unix_time_into_ntp_time
* Parameters: timeval struct and ntp_timestamp struct
* Returns: void
* Comments: takes in a pointer of a timeval and converts the ntp_timestamp to ntp time.
*           Converts from 1970 (unix timeval) to 1900 (ntp timestamp) time
*/
void convert_unix_time_into_ntp_time(struct timeval *tv, ntp_timestamp *ntp)
{
    ntp->second = tv->tv_sec + 0x83AA7E80;
    ntp->fraction = (uint32_t)( (double)(tv->tv_usec+1) * (double)(1LL<<32) * 1.0e-6 );
}

/*
* Function: getCurrentTimestamt
* Paramters: none
* Returns: an ntp_timestamp
* Comments: Gets current time using gettimeofday function and returns in NTP time
*/
ntp_timestamp getCurrentTimestamp() // Gets current time and returns as an NTP timestamp
{
  struct timeval tv;

  gettimeofday(&tv, NULL);
  ntp_timestamp returnTimeStamp;
  memset( &returnTimeStamp, 0, sizeof( ntp_timestamp ) );
  returnTimeStamp.second = tv.tv_sec + EPOCH;
  returnTimeStamp.fraction = (uint32_t)((double)(tv.tv_usec+1) * (double)(1LL<<32) * 1.0e-6);

  return returnTimeStamp;
}

/*
* Function: convertHostToNetwork
* Paramters: ntp_timestamp
* Returns: void
* Comments: converts timestamp from host to network format
*/
void convertHostToNetwork(ntp_timestamp *ntp)
{
  ntp->second = htonl(ntp->second);
  ntp->fraction = htonl(ntp->fraction);
}

/*
* Function: convertNetworkToHost
* Paramters: ntp_timestamp
* Returns: void
* Comments: converts timestamp from network to host format
*/
void convertNetworkToHost(ntp_timestamp *ntp)
{
  ntp->second = ntohl(ntp->second);
  ntp->fraction = ntohl(ntp->fraction);
}
