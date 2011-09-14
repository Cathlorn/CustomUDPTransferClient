#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/wait.h>

#include <pthread.h>

#define BUFFSIZE 320

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <fcntl.h>

#define uint32 unsigned int
#define uint16 unsigned short
#define uint8  unsigned char

static struct sockaddr_in echoserver;
static int sock;
static int runThreads = 1;
void *read_thread ( void *ptr );
//void *write_thread( void *ptr );

void ErrorHandler ( char *mess )
{
    perror ( mess );
    exit ( 1 );
}

void termination_handler ( int signum )
{
    if ( signum == SIGPIPE )
    {
        runThreads = 0;
    }
}

struct UdpCommHeader
{
    unsigned char status;
    unsigned char mgmt;
    uint16 seqNum;
    unsigned char reserved[8];
};

struct Location
{
    //double latitude;
    //double longitude;
    //double altitude;
    //double accuracy;
    uint32 latitude;
    uint32 longitude;
    uint32 altitude;
    uint16 accuracy;
};

/*struct PERS_Data
{   //Assuming 10 min worth of data
    int temp[2];
    int batt[2];
    int steps[10];
    int activity[10];
    struct Location location;
}; */

struct PERS_Data
{   //Assuming 10 min worth of data
    uint32 customerId; //Needs to be at least this size whatever we use
    uint32 time; //Needs to be at least this size whatever we use
    uint16 temp; //Same as now
    uint16 batt;  //Same as now
    uint16 steps[10]; //Same as now
    uint32 activity[10]; //Same as now
    //Location
    uint32 latitude;
    uint32 longitude;
    uint32 altitude;
    uint16 accuracy;
    uint8 reserved[2];
};

struct PERS_DataPacket
{   //Assuming 10 min 
    struct UdpCommHeader header;
    struct PERS_Data data;
};

int main ( int argc, char *argv[] )
{
    pthread_t thread1;
    char *message1 = "Thread 1";
    int  iret1;
    int  sock_flags;

    if ( argc != 3 )
    {
        fprintf ( stderr, "USAGE: simple-client <server_ip> <port>\n" );
        exit ( 1 );
    }

    //Setup signal handlers

    struct sigaction new_action; //, old_action;

    /* Set up the structure to specify the new action. */
    new_action.sa_handler = termination_handler;

    sigemptyset ( &new_action.sa_mask );

    new_action.sa_flags = 0;

    //if (old_action.sa_handler != SIG_IGN)
    sigaction ( SIGPIPE, &new_action, NULL );

    /* Create the TCP socket */
    if ( ( sock = socket ( PF_INET, SOCK_DGRAM, IPPROTO_UDP ) ) < 0 )
    {
        ErrorHandler ( "Failed to create socket" );
    }

    /* Construct the server sockaddr_in structure */
    memset ( &echoserver, 0, sizeof ( echoserver ) ); /* Clear struct */

    echoserver.sin_family = AF_INET;                  /* Internet/IP */

    echoserver.sin_addr.s_addr = inet_addr ( argv[1] );  /* IP address */

    echoserver.sin_port = htons ( atoi ( argv[2] ) ); /* server port */

/*    // Establish connection
    if ( connect ( sock,
                   ( struct sockaddr * ) &echoserver,
                   sizeof ( echoserver ) ) < 0 )
    {
        ErrorHandler ( "Failed to connect with server" );
    } */

    /* Create independent threads each of which will execute function */
    //iret1 = pthread_create ( &thread1, NULL, read_thread, ( void* ) message1 );

    // put client socket into nonblocking mode
    sock_flags = fcntl ( sock, F_GETFL, 0 );

    fcntl ( sock, F_SETFL, sock_flags | O_NONBLOCK );

    //pthread_join ( thread1, NULL );

    struct PERS_DataPacket pkt;
    struct UdpCommHeader header;

    pkt.header.status = 2;
    sendto ( sock, &pkt, sizeof(pkt), 0,
                   ( struct sockaddr * ) &echoserver,
                   sizeof ( echoserver ) );

    printf("Sent %lu bytes\n", sizeof(pkt));
    while(1)
{
    socket_read();
}

    close ( sock );

    printf ( "closing app.......\n" );

    exit ( 0 );
}


int socket_read ()
{
    char buffer[BUFFSIZE];
    int received = 0;
    int  count        = 0;
    int  actualCount  = 0;
    int  strCount     = 0;
    char tempString[1024];
    int bytes = 0;
    socklen_t echoServerAddrLength = sizeof ( struct sockaddr_in );

    bytes = recvfrom ( sock, buffer, BUFFSIZE - 1, 0, ( struct sockaddr * ) & echoserver, &echoServerAddrLength );

    //Check src here

    if ( bytes > 0 )
    {
        struct UdpCommHeader *header = (struct UdpCommHeader *) buffer;
        
        if((header->status & 1) == 1)
        {
            //Ack Received

           //Send notification of receipt of ack to server
           struct UdpCommHeader ackConfirmHeader;
           ackConfirmHeader.status = 1;
           sendto ( sock, &ackConfirmHeader, sizeof(ackConfirmHeader), 0,
                    ( struct sockaddr * ) &echoserver,
                    sizeof ( echoserver ) );
            
        }
        /*received += bytes;
        buffer[bytes] = '\0';        // Assure null terminated string

        strCount = 0;
        actualCount = 0;

        for ( count = 0; count < bytes; count++ )
        {
            if ( buffer[count] != '\0' )
            {
                tempString[strCount++] = buffer[count];
                tempString[strCount]   = '\0';
                actualCount++;
            }
        }

        if ( actualCount != 0 )
        {
            printf ( "%s", tempString );
            fflush ( stdout ); //forces the stdout output from printf out. a write command would work also or setvbuf.
        }*/
    }

    return bytes;
}
