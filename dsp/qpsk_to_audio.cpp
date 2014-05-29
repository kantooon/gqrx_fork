#include "qpsk_to_audio.h"

qpsk_to_audio_sptr
make_qpsk_to_audio ()
{
  return gnuradio::get_initial_sptr(new qpsk_to_audio);
}

qpsk_to_audio::qpsk_to_audio() :
        gr::sync_interpolator("qpsk_to_audio",
                       gr::io_signature::make (1, 1, sizeof (float)),
                       gr::io_signature::make (1, 1, sizeof (float)),2)
{
    void *tetra_tall_ctx;

    tetra_tall_ctx = NULL;
    tms = talloc_zero(tetra_tall_ctx, struct tetra_mac_state);
    tetra_mac_state_init(tms);

    tetra_gsmtap_init("localhost", 0);

    trs = talloc_zero(tetra_tall_ctx, struct tetra_rx_state);
    trs->burst_cb_priv = tms;
}

qpsk_to_audio::~qpsk_to_audio()
{
    talloc_free(trs);
    talloc_free(tms);
}

int qpsk_to_audio::work(int noutput_items,
       gr_vector_const_void_star &input_items,
       gr_vector_void_star &output_items)
{

    const float *in = (const float*)input_items[0];
    float *out = (float*)output_items[0];

    u_int8_t outbuf[noutput_items *2];
    int j =0;
    for(int i = 0; i < noutput_items; i++)
    {


        int ret;
        /* very simplistic scheme */
        if (in[i] > 2)
            ret = 3;
        else if (in[i] > 0)
            ret = 1;
        else if (in[i] < -2)
            ret = -3;
        else
            ret = -1;

        switch (ret)
        {
            case -3:

            out[j] = 0.9;
            out[j+1] = 0.9;
            outbuf[j] = 1;
            outbuf[j+1] = 1;
            //printf("11");
                break;
            case 1:

            out[j] = -0.01;
            out[j+1] = -0.01;
            outbuf[j] = 0;
            outbuf[j+1] = 0;
            //printf("00");
                break;

            case 3:
            //out[i] = 0.9;
            out[j] = -0.01;
            out[j+1] = 0.9;
            outbuf[j] = 0;
            outbuf[j+1] = 1;
            //printf("01");
                break;

            case -1:

            out[j] = 0.9;
            out[j+1] = -0.01;
            outbuf[j] = 1;
            outbuf[j+1] = 0;
            //printf("10");
                break;
            default:
                printf("null");
                break;
        }
        j=j+2;
    }


    //struct tetra_rx_state *trs = new tetra_rx_state;
    //struct tetra_mac_state *tms = new tetra_mac_state;
    //memset(trs,0, sizeof(tetra_rx_state));
    //memset(tms,0, sizeof(tetra_mac_state));

    int k = 0;





    while ((k * 64) <= (noutput_items *2)) {
        u_int8_t buf[64];


        memcpy(buf, outbuf + (k * 64), sizeof(buf));

        k++;


        tetra_burst_sync_in(trs, buf, sizeof(buf));

    }

    return noutput_items;
}
