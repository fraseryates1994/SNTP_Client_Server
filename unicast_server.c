/* Fraser Yates
*  SNTP unicast server
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

#define PORT 1236

/*
* Function : error
* Parameters: A char* message
* Returns: void
*/
void error( char* msg )
{
    perror( msg ); // Print the error message

    exit( 0 ); // Quit the process.
}

int main(int argc, char *argv[])
{
    int sockfd, n;
    struct sockaddr_in serv_addr;
    socklen_t addr_len;

    // Create and zero packet
   	ntp_packet packet;
   	memset( &packet, 0, sizeof( ntp_packet ) );

   // Create socket
    sockfd = socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );
    if (sockfd == -1) // If sockfd = -1, error opening socket
    {
        error("opening socket");
    }

    memset( &serv_addr, 0, sizeof( serv_addr)); // zero server
    serv_addr.sin_family = AF_INET; // host byte order
    serv_addr.sin_port = htons(PORT); // short, network byte order
    serv_addr.sin_addr.s_addr = INADDR_ANY; // any of server IP address

    if ( bind( sockfd, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr)) == -1)
    {
        error("binding");
    }

    addr_len = sizeof(struct sockaddr_in);

    while(1) // Loop for listening and sending
    {

      // receive packet
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

      // Set receive to current time
      packet.rxTm = getCurrentTimestamp(); // receive

      // copies transmit timestamp to originate timestamp
      packet.origTm = packet.txTm; // originate

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

      // Send packet
      n = sendto(sockfd, ( char* ) &packet, sizeof( ntp_packet), 0, (struct sockaddr *)&serv_addr, sizeof(struct sockaddr));

      // if n == -1 error writing to socket
    	if ( n == -1 )
    		error( "ERROR writing to socket" );
    }
}
