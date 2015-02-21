/* simple UDP receiver that acknowledges every datagram */

#include <cstdlib>
#include <iostream>
#include <cassert>

#include "socket.hh"
#include "contest_message.hh"
#include "timestamp.hh"

using namespace std;

#define NUM_MS_IN_TIME_SLICE      (10)
#define BW_SMOOTHING_ALPHA        (0.3f)

#define MAX(x, y) ((x > y) ? (x) : (y))
#define MIN(x, y) ((x < y) ? (x) : (y))

uint64_t current_time_slice = 0;
uint64_t num_bytes_received_in_time_slice = 0;
float estimated_bw = 5.0f;

int main( int argc, char *argv[] )
{
   /* check the command-line arguments */
  if ( argc < 1 ) { /* for sticklers */
    abort();
  }

  if ( argc != 2 ) {
    cerr << "Usage: " << argv[ 0 ] << " PORT" << endl;
    return EXIT_FAILURE;
  }

  /* create UDP socket for incoming datagrams */
  UDPSocket socket;

  /* turn on timestamps on receipt */
  socket.set_timestamps();

  /* "bind" the socket to the user-specified local port number */
  socket.bind( Address( "::0", argv[ 1 ] ) );

  cerr << "Listening on " << socket.local_address().to_string() << endl;

  uint64_t sequence_number = 0;

  /* Loop and acknowledge every incoming datagram back to its source */
  while ( true ) {
    const UDPSocket::received_datagram recd = socket.recv();

    ContestMessage message = recd.payload;

    uint64_t current_time_ms = timestamp_ms();
    uint64_t time_slice_now = current_time_ms / NUM_MS_IN_TIME_SLICE;

    assert(time_slice_now >= current_time_slice);


    float sample_bw = 5.0f;

    if(time_slice_now == current_time_slice)
    {
      uint64_t start_of_current_time_slice_ms = current_time_slice * NUM_MS_IN_TIME_SLICE;
      assert(current_time_ms >= start_of_current_time_slice_ms);
      uint64_t elapsed_time = MAX(current_time_ms - start_of_current_time_slice_ms, 1);

      //printf("message.payload.length(): %ld\n", message.payload.length());
      num_bytes_received_in_time_slice += message.payload.length();
      
      
      sample_bw = ((float) num_bytes_received_in_time_slice / (float)elapsed_time);
    }
    else
    {

      sample_bw = ((float) num_bytes_received_in_time_slice / (float)NUM_MS_IN_TIME_SLICE);

      //

      current_time_slice = time_slice_now;
      num_bytes_received_in_time_slice = message.payload.length();
    }

    estimated_bw = (BW_SMOOTHING_ALPHA * estimated_bw) + ((1.0f - BW_SMOOTHING_ALPHA) * sample_bw);

    //printf("BW @ %lu : %f\n", current_time_ms / 1000, estimated_bw  * 8 / 1024);

    /* assemble the acknowledgment */
    message.transform_into_ack( sequence_number++, recd.timestamp, estimated_bw);

    /* timestamp the ack just before sending */
    message.set_send_timestamp();

    /* send the ack */
    socket.sendto( recd.source_address, message.to_string() );
  }

  return EXIT_SUCCESS;
}
