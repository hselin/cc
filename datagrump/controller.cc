#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug )
{ debug_ = false; }





/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  //printf("this->window_size_: %d\n", this->window_size_);
  return this->window_size_;

  //return 15;
}

#if 0
/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  /* Default: fixed window size of 100 outstanding datagrams */
  unsigned int the_window_size = 50;

  if ( debug_ ) {
    cerr << "At time " << timestamp_ms()
	 << " window size is " << the_window_size << endl;
  }

  return the_window_size;
}
#endif

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp )
                                    /* in milliseconds */
{
  /* Default: take no action */

  if ( debug_ ) {
    cerr << "At time " << send_timestamp
	 << " sent datagram " << sequence_number << endl;
  }
}

#define MAX(x, y) ((x > y) ? (x) : (y))
#define MIN(x, y) ((x < y) ? (x) : (y))

/* An ack was received */
void Controller::ack_received( const uint64_t sequence_number_acked,
			       /* what sequence number was acknowledged */
			       const uint64_t send_timestamp_acked,
			       /* when the acknowledged datagram was sent (sender's clock) */
			       const uint64_t recv_timestamp_acked,
			       /* when the acknowledged datagram was received (receiver's clock)*/
			       const uint64_t timestamp_ack_received )
                               /* when the ack was received (by sender) */
{
  /* Default: take no action */

  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
	 << endl;
  }

  //uint64_t diff = (send_timestamp_acked - recv_timestamp_acked);
  uint64_t diff = (timestamp_ack_received - send_timestamp_acked);

  if( 0)
  {
    printf("DIFF: %lu\n", diff);
  }

#if 0
  if(diff < (uint64_t)70)
    this->window_size_ = MIN(this->window_size_ + 2, 22);
  else
    this->window_size_ = (unsigned int)MAX((int)(this->window_size_ - 5), 2);
#endif

#if 0
  if(diff < (uint64_t)70)
    this->window_size_ = MIN(this->window_size_ + 2, 21);
  else
    this->window_size_ = (unsigned int)MAX((int)(this->window_size_ - 5), 2);
#endif
#if 0
  if(diff < (uint64_t)70)
    this->window_size_ = MIN(this->window_size_ + 2, 21);
  else
    this->window_size_ = (unsigned int)MAX((int)(this->window_size_ - 5), 2);
#endif

  if(diff < (uint64_t)70)
    this->window_size_ = MIN(this->window_size_ + 2, 20);
  else
    this->window_size_ = (unsigned int)MAX((int)(this->window_size_ - 5), 2);

  

  if( 0)
  {
    printf("this->window_size_: %u\n", this->window_size_);
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms( void )
{
  return 800; /* timeout of one second */
}
