/* -*- c++ -*- */
/*
 * Gqrx SDR: Software defined radio receiver powered by GNU Radio and Qt
 *           http://gqrx.dk/
 *
 * Copyright 2011-2013 Alexandru Csete OZ9AEC.
 *
 * Gqrx is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * Gqrx is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Gqrx; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */
#include <iostream>
#include "receivers/nbrx.h"

#define PREF_QUAD_RATE  48000.0
#define PREF_AUDIO_RATE 48000.0
#define QPSK_QUAD_RATE  36000.0

nbrx_sptr make_nbrx(float quad_rate, float audio_rate)
{
    return gnuradio::get_initial_sptr(new nbrx(quad_rate, audio_rate));
}

nbrx::nbrx(float quad_rate, float audio_rate)
    : receiver_base_cf("NBRX"),
      d_running(false),
      d_quad_rate(quad_rate),
      d_audio_rate(audio_rate),
      d_demod(NBRX_DEMOD_FM)
{
    iq_resamp = make_resampler_cc(PREF_QUAD_RATE/d_quad_rate);
    iq_resamp_qpsk = make_resampler_cc(QPSK_QUAD_RATE/d_quad_rate);

    nb = make_rx_nb_cc(PREF_QUAD_RATE, 3.3, 2.5);
    filter = make_rx_filter(PREF_QUAD_RATE, -5000.0, 5000.0, 1000.0);
    filter_qpsk = make_rx_filter(QPSK_QUAD_RATE, -12500.0, 12500.0, 1000.0);
    agc = make_rx_agc_cc(PREF_QUAD_RATE, true, -100, 0, 2, 100, false);
    sql = gr::analog::simple_squelch_cc::make(-150.0, 0.001);
    meter = make_rx_meter_c(DETECTOR_TYPE_RMS);
    demod_ssb = gr::blocks::complex_to_real::make(1);
    demod_fm = make_rx_demod_fm(PREF_QUAD_RATE, PREF_AUDIO_RATE, 5000.0, 75.0e-6);
    demod_am = make_rx_demod_am(PREF_QUAD_RATE, PREF_AUDIO_RATE, true);
    demod_qpsk = make_rx_demod_qpsk(2,0.75,0.03,0.05,0.05,0.05, QPSK_QUAD_RATE);
    audio_rr = make_resampler_ff(d_audio_rate/PREF_AUDIO_RATE);
    audio_rr_dsd = make_resampler_ff(PREF_AUDIO_RATE/8000);
    dsd = dsd_make_block_ff();
    gain_dsd = gr::blocks::multiply_const_ff::make(3);
    float_to_char = gr::blocks::float_to_char::make(1,1.0);
    shout_streamer = make_shoutstreamer();

    connect(self(), 0, iq_resamp, 0);
    connect(iq_resamp, 0, nb, 0);
    connect(nb, 0, filter, 0);
    connect(filter, 0, meter, 0);
    connect(filter, 0, sql, 0);
    connect(sql, 0, agc, 0);
    connect(agc, 0, demod_fm, 0);
    connect(demod_fm, 0, audio_rr, 0);
    connect(audio_rr, 0, self(), 0); // left  channel
    connect(audio_rr, 0, self(), 1); // right channel
    // FIXME: we only need audio_rr when audio_rate != PREF_AUDIO_RATE

}

nbrx::~nbrx()
{

}

bool nbrx::start()
{
    d_running = true;

    return true;
}

bool nbrx::stop()
{
    d_running = false;

    return true;
}

void nbrx::set_quad_rate(float quad_rate)
{
    if (abs(d_quad_rate-quad_rate) > 0.5)
    {
#ifndef QT_NO_DEBUG_OUTPUT
        std::cout << "Changing NB_RX quad rate: "  << d_quad_rate << " -> " << quad_rate << std::endl;
#endif
        d_quad_rate = quad_rate;
        lock();
        iq_resamp->set_rate(PREF_QUAD_RATE/d_quad_rate);
        //FIXME: leaks like a sieve
        iq_resamp_qpsk = make_resampler_cc(QPSK_QUAD_RATE/quad_rate);
        unlock();
    }
}

void nbrx::set_audio_rate(float audio_rate)
{
    (void) audio_rate;
}

void nbrx::set_filter(double low, double high, double tw)
{
    filter->set_param(low, high, tw);
}

float nbrx::get_signal_level(bool dbfs)
{
    if (dbfs)
        return meter->get_level_db();
    else
        return meter->get_level();

}

void nbrx::set_nb_on(int nbid, bool on)
{
    if (nbid == 1)
        nb->set_nb1_on(on);
    else if (nbid == 2)
        nb->set_nb2_on(on);
}

void nbrx::set_nb_threshold(int nbid, float threshold)
{
    if (nbid == 1)
        nb->set_threshold1(threshold);
    else if (nbid == 2)
        nb->set_threshold2(threshold);
}

void nbrx::set_sql_level(double level_db)
{
    sql->set_threshold(level_db);
}

void nbrx::set_sql_alpha(double alpha)
{
    sql->set_alpha(alpha);
}

void nbrx::set_agc_on(bool agc_on)
{
    agc->set_agc_on(agc_on);
}

void nbrx::set_agc_hang(bool use_hang)
{
    agc->set_use_hang(use_hang);
}

void nbrx::set_agc_threshold(int threshold)
{
    agc->set_threshold(threshold);
}

void nbrx::set_agc_slope(int slope)
{
    agc->set_slope(slope);
}

void nbrx::set_agc_decay(int decay_ms)
{
    agc->set_decay(decay_ms);
}

void nbrx::set_agc_manual_gain(int gain)
{
    agc->set_manual_gain(gain);
}

void nbrx::set_demod(int rx_demod)
{
    nbrx_demod current_demod = d_demod;

    /* check if new demodulator selection is valid */
    if ((rx_demod < NBRX_DEMOD_NONE) || (rx_demod > NBRX_DEMOD_DSD))
        return;

    if (rx_demod == current_demod) {
        /* nothing to do */
        return;
    }

    /* lock graph while we reconfigure */
    lock();

    /* disconnect current demodulator */
    switch (current_demod) {

    default:
    case NBRX_DEMOD_NONE: /** FIXME! **/
    case NBRX_DEMOD_SSB:
        disconnect(agc, 0, demod_ssb, 0);
        disconnect(demod_ssb, 0, audio_rr, 0);
        break;

    case NBRX_DEMOD_AM:
        disconnect(agc, 0, demod_am, 0);
        disconnect(demod_am, 0, audio_rr, 0);
        break;

    case NBRX_DEMOD_FM:
        disconnect(agc, 0, demod_fm, 0);
        disconnect(demod_fm, 0, audio_rr, 0);
        break;

    case NBRX_DEMOD_QPSK:
        disconnect(self(), 0, iq_resamp_qpsk, 0);
        connect(self(), 0, iq_resamp, 0);
        disconnect(iq_resamp_qpsk, 0, filter_qpsk, 0);
        disconnect(filter_qpsk, 0, sql, 0);
        disconnect(filter_qpsk, 0, meter, 0);
        disconnect(sql, 0, demod_qpsk, 0);
        //disconnect(agc, 0, demod_qpsk, 0);
        disconnect(demod_qpsk, 0, self(), 0);
        disconnect(demod_qpsk, 0, self(), 1);
        connect(iq_resamp, 0, nb, 0);
        connect(sql, 0, agc, 0);
        connect(nb, 0, filter, 0);
        connect(filter, 0, sql, 0);
        connect(filter, 0, meter, 0);
        connect(audio_rr,0 ,self(), 0);
        connect(audio_rr,0 ,self(), 1);
        break;

    case NBRX_DEMOD_DSD:
        disconnect(agc, 0, demod_fm, 0);
        disconnect(demod_fm, 0, audio_rr, 0);
        disconnect(audio_rr, 0, gain_dsd, 0);
        disconnect(audio_rr_dsd, 0, self(), 0);
        disconnect(audio_rr_dsd, 0, self(), 1);
        disconnect(dsd, 0, audio_rr_dsd, 0);
        disconnect(gain_dsd, 0, dsd, 0);
        connect(audio_rr,0 ,self(), 0);
        connect(audio_rr,0 ,self(), 1);
        break;
    }

    switch (rx_demod) {

    case NBRX_DEMOD_NONE: /** FIXME! **/
    case NBRX_DEMOD_SSB:
        d_demod = NBRX_DEMOD_SSB;
        connect(agc, 0, demod_ssb, 0);
        connect(demod_ssb, 0, audio_rr, 0);
        break;

    case NBRX_DEMOD_AM:
        d_demod = NBRX_DEMOD_AM;
        connect(agc, 0, demod_am, 0);
        connect(demod_am, 0, audio_rr, 0);
        break;

    case NBRX_DEMOD_FM:
        d_demod = NBRX_DEMOD_FM;
        connect(agc, 0, demod_fm, 0);
        connect(demod_fm, 0, audio_rr, 0);
        break;
    case NBRX_DEMOD_QPSK:
        d_demod = NBRX_DEMOD_QPSK;
        disconnect(nb, 0, filter, 0);
        disconnect(filter, 0, sql, 0);
        disconnect(filter, 0, meter, 0);
        disconnect(audio_rr, 0 ,self(), 0);
        disconnect(audio_rr, 0 ,self(), 1);
        disconnect(sql, 0, agc, 0);
        disconnect(self(), 0, iq_resamp, 0);
        disconnect(iq_resamp, 0, nb, 0);
        connect(self(), 0, iq_resamp_qpsk, 0);
        connect(iq_resamp_qpsk, 0, filter_qpsk, 0);
        connect(filter_qpsk, 0, sql, 0);
        connect(filter_qpsk, 0, meter, 0);
        connect(sql, 0, demod_qpsk, 0);
        connect(demod_qpsk, 0, self(), 0);
        connect(demod_qpsk, 0, self(), 1);
        break;

    case NBRX_DEMOD_DSD:
        d_demod = NBRX_DEMOD_DSD;
        connect(agc, 0, demod_fm, 0);
        connect(demod_fm, 0, audio_rr, 0);
        disconnect(audio_rr, 0 ,self(), 0);
        disconnect(audio_rr, 0 ,self(), 1);
        connect(audio_rr, 0, gain_dsd, 0);
        connect(gain_dsd, 0, dsd, 0);
        connect(dsd, 0, audio_rr_dsd, 0);
        connect(audio_rr_dsd, 0, self(), 0);
        connect(audio_rr_dsd, 0, self(), 1);
        break;

    default:
        /* use FMN */  
        d_demod = NBRX_DEMOD_FM;
        connect(agc, 0, demod_fm, 0);
        connect(demod_fm, 0, audio_rr, 0);
        break;
    }

    /* continue processing */
    unlock();
}

void nbrx::set_fm_maxdev(float maxdev_hz)
{
    demod_fm->set_max_dev(maxdev_hz);
}

void nbrx::set_fm_deemph(double tau)
{
    demod_fm->set_tau(tau);
}

void nbrx::set_am_dcr(bool enabled)
{
    demod_am->set_dcr(enabled);
}
