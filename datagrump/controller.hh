#ifndef CONTROLLER_HH
#define CONTROLLER_HH

#include <cstdint>

/* Congestion controller interface */

#if 0
#define INITIAL_WINDOW_SIZE (5)
#endif

#define INITIAL_WINDOW_SIZE (100)
#define MAX_WINDOW_SIZE (21)

class Controller
{
private:
  bool debug_; /* Enables debugging output */

  /* Add member variables here */
  unsigned int window_size_ = INITIAL_WINDOW_SIZE;
  unsigned int max_window_size_ = MAX_WINDOW_SIZE;

  uint64_t last_sent_packet_sequence_number_ = 0;
  uint64_t prev_recv_timestamp_acked_ = 0;
  uint64_t prev_recv_sequence_number_ = 0;
  float estimated_bw_ = 0;



  uint64_t most_recent_rtt_ = 0;
  uint64_t ema_rtt_ = 0; 

public:
  /* Public interface for the congestion controller */
  /* You can change these if you prefer, but will need to change
     the call site as well (in sender.cc) */

  /* Default constructor */
  Controller( const bool debug );

  /* Get current window size, in datagrams */
  unsigned int window_size( void );

  /* A datagram was sent */
  void datagram_was_sent( const uint64_t sequence_number,
			  const uint64_t send_timestamp );

  /* An ack was received */
  void ack_received( const uint64_t sequence_number_acked,
		     const uint64_t send_timestamp_acked,
		     const uint64_t recv_timestamp_acked,
		     const uint64_t timestamp_ack_received,
         const uint64_t ack_payload_length);

  /* How long to wait (in milliseconds) if there are no acks
     before sending one more datagram */
  unsigned int timeout_ms( void );
};

#endif
