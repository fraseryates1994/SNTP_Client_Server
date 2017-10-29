/* Fraser Yates
*  SNTP manycast server
*  Date started: 15/10/2016
*  Date submitted: 07/12/2016
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ntp_time.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT 1235

/*
* Function : error
* Parameters: A char* message
* Returns: void
*/
void error( char* msg )
{
    perror( msg ); // Print the error message

    exit( 0 ); // Quit
}

/*
* Function : getReferenceTimestamp
* Parameters: None
* Returns: ntp_timestamp
* Comments: Since we are faking the synchronization of the server I am setting the reference timestamp
*           to the current time of day minus 10 minutes.
*/
ntp_timestamp getReferenceTimestamp() // Gets current time and returns as an NTP timestamp
{
  struct timeval tv;

  gettimeofday(&tv, NULL);
  ntp_timestamp returnTimeStamp;
  memset( &returnTimeStamp, 0, sizeof( ntp_timestamp ) );
  returnTimeStamp.second = (tv.tv_sec + EPOCH) - 600;
  returnTimeStamp.fraction = (uint32_t)((double)(tv.tv_usec+1) * (double)(1LL<<32) * 1.0e-6);

  return returnTimeStamp;
}

int main(int argc, char *argv[])
{
    int sockfd, n, sockoptfd, sockoptfd2;
    int manycast = 0, reuse = 1;
    struct sockaddr_in serv_addr;
    socklen_t addr_len;
    struct ip_mreq mreq;

    if (strcmp(argv[1], "m") == 0)
    {
           manycast = 1;
           printf("Entering Manycast Mode\n");
    }
    else if (strcmp(argv[1], "u") == 0)
    {
      manycast = 0;
      printf("Entering Unicast Mode\n");
    }

    // Create and zero packet
   	ntp_packet packet;
   	memset( &packet, 0, sizeof( ntp_packet ) );

    // create socket
    sockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if (sockfd == -1) // If sockfd = -1, error opening socket
    {
        error("opening socket");
    }

    memset( &serv_addr, 0, sizeof( serv_addr)); // zero server
    serv_addr.sin_family = AF_INET; // host byte order
    serv_addr.sin_port = htons(PORT); // short, network byte order

    sockoptfd = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
    if (sockoptfd == -1)
    {
      printf("Error setsockopt for reusing\n");
    }

    if ( bind( sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) == -1)
    {
        error("binding");
    }

    addr_len = sizeof(struct sockaddr_in);

    if (manycast == 1)
    {
      // Join multicast group
       mreq.imr_multiaddr.s_addr = inet_addr("224.0.1.1"); // IP of multicast group
       mreq.imr_interface.s_addr = INADDR_ANY; // local IP of interface

       sockoptfd2 = setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
       if (sockoptfd2 == -1)
       {
         error("setsockopt");
       }
    }
    else
    {
       serv_addr.sin_addr.s_addr = INADDR_ANY; // any of server IP address
    }

    while(1) // Loop for listening and sending
    {
      // Listen
      printf("Listening...\n");
      n = recvfrom( sockfd, ( char*) &packet, sizeof( ntp_packet), 0, (struct sockaddr*)&serv_addr, &addr_len);
      if (n == -1)
      {
        error("ERROR reading from socket\n");
      }

      // Check packet size is in correct format
      if (n != sizeof(packet))
      {
        printf("ERROR Packet is not 48 bytes\n");
      }

      // Set the first byte's bits to 00,100,100 for li = 0, vn = 4, and mode = 4.
      *( ( char * ) &packet + 0 ) = 0x24;

      // Convert from network to host
      convertNetworkToHost(&packet.txTm);
      convertNetworkToHost(&packet.origTm);
      convertNetworkToHost(&packet.rxTm);
      convertNetworkToHost(&packet.refTm);

      // Set receive to current time
      packet.rxTm = getCurrentTimestamp();

      // copies transmit timestamp to originate timestamp
      packet.origTm = packet.txTm;

      // Set reference timestamp
      packet.refTm = getReferenceTimestamp();

      // Set stratum to 1
      packet.stratum = 1;

      // Set reference identifier to appropriate kiss-o-death message and stratum to 0
      // kiss-o-death test
      // packet.stratum = 0;
      // strcpy(packet.refId, "AUTH");

      // Set receive and transmit timestamp to current timestamp
      packet.txTm = getCurrentTimestamp(); // transmit

      // Convert from host to network
      convertHostToNetwork(&packet.txTm);
      convertHostToNetwork(&packet.origTm);
      convertHostToNetwork(&packet.rxTm);
      convertHostToNetwork(&packet.refTm);

      // Send packet
      n = sendto(sockfd, ( char* ) &packet, sizeof( ntp_packet), 0, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr));
      printf("Sent to: %s\n", inet_ntoa(serv_addr.sin_addr));

      // if n == -1 error writing to socket
    	if ( n == -1 )
      {
    		error( "ERROR writing to socket" );
      }
    }
}
