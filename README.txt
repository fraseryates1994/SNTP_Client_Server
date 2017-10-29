To compile the manycast server and client include the ntp_time.c file upon compiling.
For example:
gcc -Wall unicast_client.c ntp_time.c -o unicast_client
gcc -Wall manycast_server.c ntp_time.c -o manycast_server

Once compiled the client takes the URL and port number as arguments.
For example:
./unicast_client ntp.uwe.ac.uk 123
./unicast_client localhost 1235 (my server port is 1235)
./unicast_client 224.0.1.1 1235

Once compiled the server takes "m" for manycast and "u" for unicast as arguments.
For example:
./manycast_server u (this will enter unicast mode)
./manycast_server m (this will enter manycast mode)

I have inluded my unicast_server code only for history. The manycast server incorporates both unicast and manycast modes, therefore the unicast_server can be ignored.

The design test plan were handed in with the worksheets. However the wireshark screenshots  and astah diagrams can also be found on gitlab.