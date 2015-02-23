#ifndef CONTROLLER_HH
#define CONTROLLER_HH

#include <cstdint>

/* Congestion controller interface */

#define INITIAL_WINDOW_SIZE (1)


class Controller
{
private:
  bool debug_; /* Enables debugging output */

  /* Add member variables here */
  unsigned int window_size_ = INITIAL_WINDOW_SIZE;
  uint64_t last_sent_packet_sequence_number_ = 0;
  uint64_t amount_of_bytes_sent_ = 0;
  uint64_t amount_of_bytes_received_ = 0;
  uint64_t rtt_estimate_ = 100;
  uint64_t last_sent_packet_time_ = 0;
  uint64_t to_time_ = 1;

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
         const float estimated_bw);

  /* How long to wait (in milliseconds) if there are no acks
     before sending one more datagram */
  unsigned int timeout_ms( void );

  bool send_datagram(void);
};

#endif
