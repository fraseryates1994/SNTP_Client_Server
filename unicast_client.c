/* Fraser Yates
*  SNTP unicast client
*  Date started: 15/10/2016
*  Date submitted: 07/12/2016
*/
#include <stdio.h>
#include <stdlib.h>
#include "ntp_time.h"
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <netinet/in.h>

#define EPOCH 2208988800ull

/*
* Function : error
* Parameters: A char* message
* Returns: void
*/
void error( char* msg )
{
  perror( msg ); // Print error msg
  exit( 0 );
}

/*
* Function: roundTripDelay
* Paramters: 4 timevals dxTm, origTm, txTm, rxTm
* Returns: a double of the roundtrip delay
* Comments: Formula found from RFC
*/
double roundTripDelay(struct timeval dxTm,struct timeval origTm,struct timeval txTm,struct timeval rxTm)
{
  double ret, ret1, ret2;
  ret1 = (double)((dxTm.tv_sec - origTm.tv_sec) - (txTm.tv_sec - rxTm.tv_sec));
  ret2 = (double)((dxTm.tv_usec - origTm.tv_usec) - (txTm.tv_usec - rxTm.tv_usec));

  ret2 = ret2 / 1000000;
  ret = ret1 + ret2;
  return ret;
}

/*
* Function: systemClockOffset
* Paramters: 4 timevals dxTm, origTm, txTm, rxTm
* Returns: a double of the systemClockOffset
* Comments: Formula found from RFC
*/
double systemClockOffset(struct timeval rxTm,struct timeval origTm,struct timeval txTm,struct timeval dxTm)
{
  double ret, ret1, ret2;
  ret1 = (double)((rxTm.tv_sec - origTm.tv_sec) + (txTm.tv_sec - dxTm.tv_sec));
  ret2 = (double)((rxTm.tv_usec - origTm.tv_usec) + (txTm.tv_usec - dxTm.tv_usec));

  ret2 = ret2 / 1000000;
  ret = ret1 + ret2;
  return ret;
}

int main( int argc, char* argv[ ] )
{
  int sockfd, n; // Socket file descriptor, what socket() returns. and the n return result from writing/reading from the socket.
  char* host_name; // server host-name.
  int port;
  struct sockaddr_in serv_addr; // Server address data structure.
  struct hostent *server; // Server data structure.
  socklen_t addr_len;

  // Assign host name to command line argument
  if (argc != 0)
  {
    host_name = argv[1];
    port = atoi(argv[2]);
  }

  // If no hostname provided, print error
  if (argc < 2)
  {
    error("Hostname & port not provided");
  }

  // Create and zero packet
  ntp_packet packet;
  memset( &packet, 0, sizeof( ntp_packet ) );

  // Set the first byte's bits to 00,100,011 for li = 0, vn = 4, and mode = 3
  *( ( char * ) &packet + 0 ) = 0x23;

  // Create UDP socket
  printf("Setting up socket PORT %d .. ",port);
  sockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
  // If server replies -1 there is an error opening socket
  if ( sockfd < 0 )
  {
	error( "ERROR opening socket" );
  }
  printf("Done\n");

  // Convert URL to IP address and save onto server
  printf("Setting up hostname %s .. ",host_name);
  server = gethostbyname(host_name);
  if ( server == NULL )
  {
	error( "ERROR, no such host" );
  }
  printf("Done\n");

  // zero out server addr
  bzero( ( char* ) &serv_addr, sizeof( serv_addr ) );
  
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons( port ); // Set port number from host to network

  // Copy the server's IP address to the server address structure.
  bcopy( ( char* )server->h_addr, ( char* ) &serv_addr.sin_addr.s_addr, server->h_length );

  // Set transmit time to current time
  packet.txTm = getCurrentTimestamp();

  // Convert from host to network
  convertHostToNetwork(&packet.txTm);
  
  // Send packet
  n = sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr));

  // if n == 1 error writing to socket
  if ( n == -1 )
  {
    error( "ERROR writing to socket" );
  }

  // receive packet back from server
  addr_len = sizeof( struct sockaddr);
  n = recvfrom( sockfd, &packet, sizeof(packet), 0, (struct sockaddr*)&serv_addr, &addr_len);
  
  // if n == -1 error receiving from socket
  if ( n == -1 )
  {
	error( "ERROR reading from socket" );
  }

  // Check packet size is in correct format
  if (n != sizeof(packet))
  {
    printf("ERROR Packet is expected size\n");
  }

  // Convert from network to host
  convertNetworkToHost(&packet.txTm);
  convertNetworkToHost(&packet.origTm);
  convertNetworkToHost(&packet.rxTm);

  // Create destination timestamp and convert to UNIX timeval for caluclations
  ntp_timestamp dxTm;
  dxTm = getCurrentTimestamp();

  // check for kiss-o-death packet
  if (packet.stratum == 0)
  {
    printf("Kiss-o-death packet received...\nKiss-code: %s\n",packet.refId);
  }
  
  // Convert from NTP to UNIX for delay and offset calculations
  struct timeval tv, tv2, tv3, tv4;
  convert_ntp_time_into_unix_time(&dxTm,&tv4);
  convert_ntp_time_into_unix_time(&packet.txTm,&tv);
  convert_ntp_time_into_unix_time(&packet.rxTm,&tv2);
  convert_ntp_time_into_unix_time(&packet.origTm,&tv3);

  // Calculate roundtrip delay
  double roundTrip = roundTripDelay(tv4,tv3,tv,tv2);

  // Calculate system clock offset
  double offset = systemClockOffset(tv2,tv3,tv,tv4);

  // Convert to timeval and print
  time_t txTm_sec = tv.tv_sec;
  printf( "Time: %sMicroseconds: %ld\n", ctime( (const time_t* ) &txTm_sec),tv.tv_usec);
  printf("total delay: %lf\n",roundTrip);
  printf("total offset: %lf\n",offset);

  return 0;
}
