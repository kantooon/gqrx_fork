#include "rx_demod_qpsk.h"



rx_demod_qpsk_sptr make_rx_demod_qpsk(int sps, float excess_bw, float costas_alpha, float gain_mu, float mu, double omega_relative_limit, float quad_rate)
{
    return gnuradio::get_initial_sptr(new rx_demod_qpsk(sps, excess_bw, costas_alpha, gain_mu, mu, omega_relative_limit, quad_rate));
}

static const int MIN_IN = 1;  /* Mininum number of input streams. */
static const int MAX_IN = 1;  /* Maximum number of input streams. */
static const int MIN_OUT = 1; /* Minimum number of output streams. */
static const int MAX_OUT = 1; /* Maximum number of output streams. */

rx_demod_qpsk::rx_demod_qpsk(int sps, float excess_bw, float costas_alpha, float gain_mu, float mu, double omega_relative_limit, float quad_rate)
      : gr::hier_block2 ("rx_demod_qpsk",
                        gr::io_signature::make (MIN_IN, MAX_IN, sizeof (gr_complex)),
                        gr::io_signature::make (MIN_OUT, MAX_OUT, sizeof (float))),
        _samples_per_symbol(sps),
        _excess_bw(excess_bw),
        _costas_alpha(costas_alpha),
        _gain_mu(gain_mu),
        _mu(mu),
        _omega_relative_limit(omega_relative_limit),
        _sample_rate(quad_rate)
{

    int symbol_rate = 18000;

    float scale = (1.0/16384.0);
    pre_scaler = gr::blocks::multiply_const_cc::make(scale);   // scale the signal from full-range to +-1

    agc = gr::analog::feedforward_agc_cc::make(16, 2.0);


    int ntaps = 11 * _samples_per_symbol;
    std::vector<float> rrc_taps = gr::filter::firdes::root_raised_cosine(
                1.0,                      // gain
                _sample_rate,
                symbol_rate, // sampling rate
                //1.0,                      // symbol rate
                _excess_bw,          // excess bandwidth (roll-off factor)
                ntaps);
    rrc_filter = gr::filter::interp_fir_filter_ccf::make(1, rrc_taps);

    int omega = _samples_per_symbol;
    double gain_omega = 0.25 * _gain_mu * _gain_mu;
    double costas_beta  = 0.25 * _costas_alpha * _costas_alpha;
    float fmin = -0.025;
    float fmax = 0.025;

    receiver = gr::digital::mpsk_receiver_cc::make(4, M_PI/4.0, //mod order, const shift
                  2*M_PI/150,
                  fmin, fmax,
                  _mu, _gain_mu,
                  omega, gain_omega,
                  _omega_relative_limit);
    receiver->set_alpha(_costas_alpha);
    receiver->set_beta(costas_beta);

    // Perform Differential decoding on the constellation
    diffdec = gr::digital::diff_phasor_cc::make();

    // take angle of the difference (in radians)
    to_float = gr::blocks::complex_to_arg::make();

    // convert from radians such that signal is in -3/-1/+1/+3
    rescale = gr::blocks::multiply_const_ff::make( 1 / (M_PI / 4) );

    to_audio = make_qpsk_to_audio();
    //connect(self(),0,resampler_cc,0);
    //connect(resampler_cc,0,pre_scaler,0);
    connect(self(),0,pre_scaler,0);
    connect(pre_scaler,0,agc,0);
    connect(agc,0,rrc_filter,0);
    connect(rrc_filter,0,receiver,0);
    connect(receiver,0,diffdec,0);
    connect(diffdec,0,to_float,0);
    connect(to_float,0,rescale,0);
    connect(rescale,0,to_audio,0);
    connect(to_audio, 0, self(), 0);
}

void rx_demod_qpsk::set_input_rate(float quad_rate)
{
    _sample_rate = quad_rate;
}
