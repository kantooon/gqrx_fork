#ifndef QPSK_TO_AUDIO_H
#define QPSK_TO_AUDIO_H

#include <gnuradio/sync_block.h>
#include <gnuradio/sync_interpolator.h>
#include <gnuradio/io_signature.h>
#include <stdio.h>
extern "C" {
#include <osmocom/core/utils.h>
#include <osmocom/core/talloc.h>
#include <tetra_common.h>
#include "tetra_gsmtap.h"
#include <phy/tetra_burst_sync.h>
#include <phy/tetra_burst.h>

}



class qpsk_to_audio;

typedef boost::shared_ptr<qpsk_to_audio> qpsk_to_audio_sptr;

qpsk_to_audio_sptr make_qpsk_to_audio();

class qpsk_to_audio : public gr::sync_interpolator
{
public:
    qpsk_to_audio();
    ~qpsk_to_audio();

    int work(int noutput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);

private:
    struct tetra_rx_state *trs;
    struct tetra_mac_state *tms;
};

#endif // QPSK_TO_AUDIO_H
