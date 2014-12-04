#include "qpsk_to_audio.h"

qpsk_to_audio_sptr
make_qpsk_to_audio ()
{
  return gnuradio::get_initial_sptr(new qpsk_to_audio);
}

qpsk_to_audio::qpsk_to_audio() :
        gr::sync_block("qpsk_to_audio",
                       gr::io_signature::make (1, 1, sizeof (float)),
                       gr::io_signature::make (1, 1, sizeof (float)))
{


    tetra_tall_ctx = NULL;

    tetra_gsmtap_init("localhost", 0);
    tms = talloc_zero(tetra_tall_ctx, struct tetra_mac_state);
    tetra_mac_state_init(tms);

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

    const float *in = reinterpret_cast<const float*>(input_items[0]);
    float *out = reinterpret_cast<float*>(output_items[0]);

    u_int8_t outbuf[noutput_items *2];
    int j =0;
    for(int i = 0; i < noutput_items; i++)
    {

        int ret;
        if (in[i] > 2.0)
            ret = 3;
        else if (in[i] > 0.0)
            ret = 1;
        else if (in[i] < -2.0)
            ret = -3;
        else
            ret = -1;

        switch (ret)
        {
            case -3:

            out[i] = 0.5;
            outbuf[j] = 1;
            outbuf[j+1] = 1;
                break;
            case 1:

            out[i] = -0.25;
            outbuf[j] = 0;
            outbuf[j+1] = 0;
                break;

            case 3:
            out[i] = -0.5;
            outbuf[j] = 0;
            outbuf[j+1] = 1;
                break;

            case -1:

            out[i] = 0.25;
            outbuf[j] = 1;
            outbuf[j+1] = 0;
                break;
            default:
                printf("null");
                break;
        }
        j=j+2;
    }

    int k = 0;

    while ((k * 64) <= (noutput_items *2)) {
        u_int8_t buf[64];
        memset(buf,0, sizeof(buf));

        int rest = (noutput_items *2) - (k * 64);
        if(rest >= 64)
        {
            memcpy(buf, outbuf + (k * 64), sizeof(buf));
            tetra_burst_sync_in(trs, buf, sizeof(buf));
        }
        else
        {
            memcpy(buf, outbuf + (k * 64), rest);
            tetra_burst_sync_in(trs, buf, rest);
        }
        k++;

    }


    return noutput_items;
}
