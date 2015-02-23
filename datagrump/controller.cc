#include <iostream>
#include <cassert>

#include "controller.hh"
#include "timestamp.hh"

using namespace std;

/* Default constructor */
Controller::Controller( const bool debug )
  : debug_( debug )
{}

#define MAX(x, y) ((x > y) ? (x) : (y))
#define MIN(x, y) ((x < y) ? (x) : (y))
#define DIV_ROUND_UP(n,d) (((n) + (d) - 1) / (d))

#define TARGET_MAX_LATENCY      (70.0f)
#define RTT_SMOOTHING_ALPHA     (0.3f)
#define PACKET_PAYLOAD_SIZE     (1424)

/* Get current window size, in datagrams */
unsigned int Controller::window_size( void )
{
  if ( debug_ )
  {
    cerr << "At time " << timestamp_ms()
         << " window size is " << this->window_size_ << endl;
  }

  /*
  if(last_sent_packet_time_ > timestamp_ms() + 10)
  {
    return this->window_size_ + 1000;
  }
  */

  return 0;

  return this->window_size_;
}

/* A datagram was sent */
void Controller::datagram_was_sent( const uint64_t sequence_number,
				    /* of the sent datagram */
				    const uint64_t send_timestamp )
                                    /* in milliseconds */
{
  if ( debug_ ) {
    cerr << "At time " << send_timestamp
	 << " sent datagram " << sequence_number << endl;
  }

  this->last_sent_packet_sequence_number_ = MAX(this->last_sent_packet_sequence_number_, sequence_number);
  this->amount_of_bytes_sent_ += PACKET_PAYLOAD_SIZE;
  this->last_sent_packet_time_ = send_timestamp;
}

/* An ack was received */
void Controller::ack_received( const uint64_t sequence_number_acked,
			       /* what sequence number was acknowledged */
			       const uint64_t send_timestamp_acked,
			       /* when the acknowledged datagram was sent (sender's clock) */
			       const uint64_t recv_timestamp_acked,
			       /* when the acknowledged datagram was received (receiver's clock)*/
			       const uint64_t timestamp_ack_received,
                               /* when the ack was received (by sender) */
             const float estimated_bw)
{
  /* Default: take no action */

  if ( debug_ ) {
    cerr << "At time " << timestamp_ack_received
	 << " received ack for datagram " << sequence_number_acked
	 << " (send @ time " << send_timestamp_acked
	 << ", received @ time " << recv_timestamp_acked << " by receiver's clock)"
	 << endl;
  }

  assert(this->last_sent_packet_sequence_number_ >= sequence_number_acked);

  uint64_t rtt_sample;

  rtt_sample = (timestamp_ack_received - send_timestamp_acked);

  this->rtt_estimate_ = (RTT_SMOOTHING_ALPHA * this->rtt_estimate_) + ((1.0f - RTT_SMOOTHING_ALPHA) * rtt_sample);

  this->amount_of_bytes_received_ += PACKET_PAYLOAD_SIZE;

  assert(this->amount_of_bytes_sent_ >= this->amount_of_bytes_received_);

  uint64_t outstanding_data_capacity = (TARGET_MAX_LATENCY * estimated_bw);
  uint64_t outstanding_packet_capacity = outstanding_data_capacity / PACKET_PAYLOAD_SIZE;  
  
  //uint64_t num_outstanding_bytes = this->amount_of_bytes_sent_ - this->amount_of_bytes_received_;
  //uint64_t num_outstanding_packets = DIV_ROUND_UP(num_outstanding_bytes, PACKET_PAYLOAD_SIZE);
  uint64_t processed_data_capacity = (this->rtt_estimate_ / 2 * estimated_bw);
  uint64_t processed_data_packets = processed_data_capacity / PACKET_PAYLOAD_SIZE;
  //uint64_t new_window_size = 0;

  printf("estimated_bw: %f\n", estimated_bw);
#if 0
  printf("outstanding_data_capacity: %lu\n", outstanding_data_capacity);
  printf("outstanding_packet_capacity: %lu\n", outstanding_packet_capacity);
  printf("num_outstanding_bytes: %lu\n", num_outstanding_bytes);  
  printf("num_outstanding_packets: %lu\n", num_outstanding_packets);
  printf("this->last_sent_packet_sequence_number_: %lu\n", this->last_sent_packet_sequence_number_);
#endif

  outstanding_packet_capacity += processed_data_packets;
#if 0
  if(outstanding_packet_capacity > num_outstanding_packets)
  {
    //this->window_size_ = MAX((int)((outstanding_packet_capacity - num_outstanding_packets) - 1), 2); 
    //this->window_size_ += MIN((outstanding_packet_capacity - num_outstanding_packets) / 2, 2);
    //this->window_size_ += 5; //3 and 5 is a good number?
    //this->window_size_ = (outstanding_packet_capacity - num_outstanding_packets);
    printf("+ %lu\n", MAX((outstanding_packet_capacity - num_outstanding_packets), 1));
    this->window_size_ += MAX((outstanding_packet_capacity - num_outstanding_packets), 1);
    //new_window_size = (outstanding_packet_capacity - num_outstanding_packets);
  }
  else
  {
    //this->window_size_ = MAX((int)(this->window_size_ - 5), 2); //(num_outstanding_packets - outstanding_packet_capacity);
    //this->window_size_ = MAX((int)((num_outstanding_packets - outstanding_packet_capacity) - 1), 1); 
    //this->window_size_ = MAX((int)(this->window_size_ - (num_outstanding_packets - outstanding_packet_capacity)), 0);
    printf("- 0\n");
    this->window_size_ = 0;
  }
#endif

  if(outstanding_packet_capacity > this->window_size_)
  {
    //this->window_size_ = MAX((int)((outstanding_packet_capacity - num_outstanding_packets) - 1), 2); 
    //this->window_size_ += MIN((outstanding_packet_capacity - num_outstanding_packets) / 2, 2);
    //this->window_size_ += 5; //3 and 5 is a good number?
    //this->window_size_ = (outstanding_packet_capacity - num_outstanding_packets);
    //printf("+ %lu\n", MAX((outstanding_packet_capacity - this->window_size_), 1));
    this->window_size_ += MAX((outstanding_packet_capacity - this->window_size_), 1);
    //new_window_size = (outstanding_packet_capacity - num_outstanding_packets);
  }
  else
  {
    //this->window_size_ = MAX((int)(this->window_size_ - 5), 2); //(num_outstanding_packets - outstanding_packet_capacity);
    //this->window_size_ = MAX((int)((num_outstanding_packets - outstanding_packet_capacity) - 1), 1); 
    //this->window_size_ = MAX((int)(this->window_size_ - (num_outstanding_packets - outstanding_packet_capacity)), 0);
    //printf("- 0\n");
    this->window_size_ = 0;
  }
  //printf("moo: %d\n", (unsigned int) (PACKET_PAYLOAD_SIZE / estimated_bw));
  this->to_time_ = MIN((unsigned int) (PACKET_PAYLOAD_SIZE / estimated_bw), 5) + 10;
  printf("this->to_time_: %lu\n", this->to_time_);

  //this->window_size_ = (0.8 * this->window_size_) + ((1.0f - 0.8) * new_window_size);

}

/* How long to wait (in milliseconds) if there are no acks
   before sending one more datagram */
unsigned int Controller::timeout_ms( void )
{
  return (unsigned int)this->to_time_;
  //return (unsigned int)this->rtt_estimate_;
}
