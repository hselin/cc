qwe
    //uint64_t start_of_current_time_slice_ms = current_time_slice * NUM_MS_IN_TIME_SLICE;
    //uint64_t start_of_next_time_slice_ms = start_of_current_time_slice_ms + NUM_MS_IN_TIME_SLICE;

#if 0  
  float sample_bw = 0;
  uint64_t target_amount_of_outstanding_data = 0;
  uint64_t target_amount_of_outstanding_packets = 0;
  uint64_t num_outstanding_packets = 0;
  uint64_t target_window_size = 0;

  uint64_t start_of_current_time_slice = this->current_time_slice_ * TIME_SLICE_AMOUNT;
  uint64_t start_of_next_time_slice = start_of_current_time_slice + TIME_SLICE_AMOUNT;

  if(recv_timestamp_acked < start_of_current_time_slice)
  {
    printf("!!!!!!!!!!!!!!! %lu %lu\n", recv_timestamp_acked, start_of_current_time_slice);
    return;
  }

  if(recv_timestamp_acked < start_of_next_time_slice)
  {
    uint64_t ms_spent_in_current_time_slice = 0;

    //still within current time slice
    ms_spent_in_current_time_slice = recv_timestamp_acked - start_of_current_time_slice;
    this->bytes_sent_in_current_time_slice_ += ack_payload_length;

    printf("[%lu | %lu]\n", this->bytes_sent_in_current_time_slice_, ms_spent_in_current_time_slice);

    if(ms_spent_in_current_time_slice == 0)
    {
      sample_bw = 5500.00f;
    }
    else
    {
      sample_bw = ((float) this->bytes_sent_in_current_time_slice_ / (float)ms_spent_in_current_time_slice);
    }
  }
  else
  {
    uint64_t new_time_slice = recv_timestamp_acked / TIME_SLICE_AMOUNT;
    uint64_t start_of_new_time_slice = new_time_slice * TIME_SLICE_AMOUNT;
    uint64_t ms_spent_in_new_time_slice = recv_timestamp_acked - start_of_new_time_slice;
    uint64_t elapsed_time_slice = new_time_slice - this->current_time_slice_;
    uint64_t total_elapsed_time = 0;
    uint64_t total_bytes_sent = 0;

    //assert(new_time_slice > this->current_time_slice_);
    
    if(elapsed_time_slice > 1)
    {
      printf("@@@@@@@@@\n");

      total_elapsed_time = (elapsed_time_slice * TIME_SLICE_AMOUNT) + ms_spent_in_new_time_slice;
      total_bytes_sent = ack_payload_length + this->bytes_sent_in_current_time_slice_; 

    }
    else
    {
      total_elapsed_time = ms_spent_in_new_time_slice;
      total_bytes_sent = ack_payload_length; 
    }


    this->current_time_slice_ = new_time_slice;
    this->bytes_sent_in_current_time_slice_ = ack_payload_length;

    if(total_elapsed_time == 0)
    {
      sample_bw = 5500.00f;
    }
    else
    {
      sample_bw = ((float) total_bytes_sent / (float)total_elapsed_time);
    }
  }

#endif




  #if 0

  if(this->prev_recv_timestamp_acked_)
  {
    elapsed_time = recv_timestamp_acked - this->prev_recv_timestamp_acked_;
  }

  this->prev_recv_timestamp_acked_ = recv_timestamp_acked;

  //elapsed_time = (timestamp_ack_received - send_timestamp_acked) / 2;


  if(elapsed_time == 0)
  {
    if(this->estimated_bw_ == 0)
      sample_bw = 2000.00f;
    else
      sample_bw = this->estimated_bw_ ;

    sample_bw = 5500.00f;
  }
  else
  {
    sample_bw = ((float) ack_payload_length / (float)elapsed_time);
  }
  #endif

  this->estimated_bw_ = (BW_SMOOTHING_ALPHA * this->estimated_bw_) + ((1.0f - BW_SMOOTHING_ALPHA) * sample_bw);

  //printf("ack_payload_length = %lu elapsed_time = %f\n", ack_payload_length, elapsed_time);


  printf("%lu,%f\n", timestamp_ack_received / 1000, this->estimated_bw_ * (float)8 / (float)1000000);


  //printf("bw [%f, %f]\n", sample_bw, this->estimated_bw_);


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

#if 1
  if(this->window_size_ > target_window_size)
  {
    //printf("DEC: %lu\n", MAX(((this->window_size_ - target_window_size) / 2), 3));
    this->window_size_ -= MIN(((this->window_size_ - target_window_size) / 2), this->window_size_);
  }
  else
  {
    //printf("INC: %lu\n", MAX(((target_window_size - this->window_size_) / 2), 5));
    this->window_size_ += MAX(((target_window_size - this->window_size_) / 2), 10);
  }
#endif

#if 0
  if(this->window_size_ < 2)
    this->window_size_ = 2;
  
  if(this->window_size_ >= 20)
    this->window_size_ = 20;
#endif

  //this->window_size_ = target_window_size;

  //printf("this->window_size_: %u\n", this->window_size_);


  this->window_size_ = MAX(this->window_size_, 10);

  if(0)
  {
    //printf("this->most_recent_rtt_: %lu\n", diff);
  }


  if(0)
  {
    printf("this->window_size_: %u\n", this->window_size_);
  }
