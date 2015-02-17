#include <iostream>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug )
{ debug_ = false; }

#define MAX(x, y) ((x > y) ? (x) : (y))
#define MIN(x, y) ((x < y) ? (x) : (y))

#define TARGET_MAX_LATENCY      (80.0f)
#define BW_SMOOTHING_ALPHA      (0.05f)


/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  printf("this->window_size_: %d\n", this->window_size_);

  //return 25;
  return this->window_size_;
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

  if ( 0 ) {
    cerr << "At time " << send_timestamp
	 << " sent datagram " << sequence_number << endl;
  }

  this->last_sent_packet_sequence_number_ = sequence_number;
}


#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

/* An ack was received */
void Controller::ack_received( const uint64_t sequence_number_acked,
			       /* what sequence number was acknowledged */
			       const uint64_t send_timestamp_acked,
			       /* when the acknowledged datagram was sent (sender's clock) */
			       const uint64_t recv_timestamp_acked,
			       /* when the acknowledged datagram was received (receiver's clock)*/
			       const uint64_t timestamp_ack_received,
                               /* when the ack was received (by sender) */
             const uint64_t ack_payload_length)
{
  /* Default: take no action */

  if ( 0 ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
   << " with payload "<< ack_payload_length
	 << endl;
  }

  //this->most_recent_rtt_ = (timestamp_ack_received - send_timestamp_acked);

  uint64_t elapsed_time = 0;
  float sample_bw = 0;
  uint64_t target_amount_of_outstanding_data = 0;
  uint64_t target_amount_of_outstanding_packets = 0;
  uint64_t num_outstanding_packets = 0;
  uint64_t target_window_size = 0;


  if(this->prev_recv_timestamp_acked_)
  {
    elapsed_time = recv_timestamp_acked - this->prev_recv_timestamp_acked_;
  }

  this->prev_recv_timestamp_acked_ = recv_timestamp_acked;

  if(elapsed_time == 0)
  {
    sample_bw = 500.00f;
  }
  else
  {
    sample_bw = ((float) ack_payload_length / (float)elapsed_time);
  }

  this->estimated_bw_ = (BW_SMOOTHING_ALPHA * this->estimated_bw_) + ((1.0f - BW_SMOOTHING_ALPHA) * sample_bw);

  //printf("ack_payload_length = %lu elapsed_time = %f\n", ack_payload_length, elapsed_time);




  printf("bw [%f, %f]\n", sample_bw, this->estimated_bw_);


  target_amount_of_outstanding_data = (TARGET_MAX_LATENCY * this->estimated_bw_);

  
  //printf("target_amount_of_outstanding_data [%lu]\n", target_amount_of_outstanding_data);

  target_amount_of_outstanding_packets = DIV_ROUND_UP(target_amount_of_outstanding_data, ack_payload_length);

  //printf("target_amount_of_outstanding_packets [%lu]\n", target_amount_of_outstanding_packets);

  /*
  if(diff < TARGET_MAX_LATENCY)
  {
    this->window_size_ = (unsigned int) MIN(this->window_size_ + 1, 40);
  }
  else
  {
    this->window_size_ = (unsigned int) MAX((int)(this->window_size_ - 5), 5);
  }
  */

  //target_amount_of_outstanding_packets = 25;



  //this->window_size_ = target_amount_of_outstanding_packets;

  num_outstanding_packets = this->last_sent_packet_sequence_number_ - sequence_number_acked;

  //printf("[%lu, %lu]\n", this->last_sent_packet_sequence_number_, sequence_number_acked);


  //printf("num_outstanding_packets: %lu\n", num_outstanding_packets);

  //printf("[%lu, %lu]\n", num_outstanding_packets, target_amount_of_outstanding_packets);


  if(num_outstanding_packets >= target_amount_of_outstanding_packets)
  {
    target_window_size = 2;
  }
  else
  {
    target_window_size = target_amount_of_outstanding_packets - num_outstanding_packets;
  }

#if 0
  if(this->window_size_ > target_window_size)
  {
    printf("DEC: %lu\n", ((this->window_size_ - target_window_size) / 2));
    this->window_size_ -= ((this->window_size_ - target_window_size) / 2);
  }
  else
  {
    printf("INC: %lu\n", ((target_window_size - this->window_size_) / 5));
    this->window_size_ += ((target_window_size - this->window_size_) / 5);
  }
#endif

#if 0
  if(this->window_size_ < 2)
    this->window_size_ = 2;
  
  if(this->window_size_ >= 20)
    this->window_size_ = 20;
#endif

  this->window_size_ = target_window_size;

  if(0)
  {
    //printf("this->most_recent_rtt_: %lu\n", diff);
  }


  if(0)
  {
    printf("this->window_size_: %u\n", this->window_size_);
  }
}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms( void )
{

  if( 0)
  {
    printf("timeout_ms: %u\n", 0);
  }

  return 250; /* timeout of one second */
}
