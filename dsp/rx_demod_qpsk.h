#ifndef RX_DEMOD_QPSK_H
#define RX_DEMOD_QPSK_H

#include <gnuradio/hier_block2.h>
#include <gnuradio/blocks/multiply_const_cc.h>
#include <gnuradio/analog/feedforward_agc_cc.h>
#include <gnuradio/filter/iir_filter_ffd.h>
#include <gnuradio/filter/pfb_arb_resampler_ccf.h>
#include <gnuradio/io_signature.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/filter/interp_fir_filter_ccf.h>
#include <gnuradio/digital/mpsk_receiver_cc.h>
#include <gnuradio/digital/diff_phasor_cc.h>
#include <gnuradio/blocks/complex_to_arg.h>
#include <gnuradio/blocks/multiply_const_ff.h>
#include <gnuradio/filter/pfb_decimator_ccf.h>
#include "resampler_xx.h"
#include "qpsk_to_audio.h"
#include <vector>
#include <math.h>

class rx_demod_qpsk;
typedef boost::shared_ptr<rx_demod_qpsk> rx_demod_qpsk_sptr;

rx_demod_qpsk_sptr make_rx_demod_qpsk(int sps, float excess_bw, float costas_alpha, float gain_mu, float mu, double omega_relative_limit);

class rx_demod_qpsk : public gr::hier_block2
{
public:
    rx_demod_qpsk(int sps, float excess_bw, float costas_alpha, float gain_mu, float mu, double omega_relative_limit);
    void set_input_rate(float quad_rate);
private:
    float _sample_rate;
    int _samples_per_symbol = 10;
    float _excess_bw = 0.35;
    bool _gray_code = true;

    float _costas_alpha = 0.15;
    float _gain_mu = 0.0;
    float _mu = 0.5;
    double _omega_relative_limit = 0.005;

    gr::filter::pfb_decimator_ccf::sptr resamp;
    gr::blocks::multiply_const_cc::sptr pre_scaler;
    gr::analog::feedforward_agc_cc::sptr agc;
    gr::filter::interp_fir_filter_ccf::sptr rrc_filter;
    gr::digital::mpsk_receiver_cc::sptr receiver;
    gr::digital::diff_phasor_cc::sptr diffdec;
    gr::blocks::complex_to_arg::sptr to_float;
    gr::blocks::multiply_const_ff::sptr rescale;
    resampler_cc_sptr resampler_cc;
    qpsk_to_audio_sptr to_audio;
};

#endif // RX_DEMOD_QPSK_H
