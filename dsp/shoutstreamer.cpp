#include "shoutstreamer.h"


shoutstreamer_sptr
make_shoutstreamer()
{
  return gnuradio::get_initial_sptr(new ShoutStreamer);
}



ShoutStreamer::ShoutStreamer() :
        gr::sync_block("shoutstreamer",
               gr::io_signature::make (1, 1, sizeof (char)),
               gr::io_signature::make (0, 0, 0))
{
    shout_init();
    _shout_stream = shout_new();
    shout_set_host(_shout_stream,"localhost");
    shout_set_agent(_shout_stream,"gqrx");
    shout_set_mount(_shout_stream,"/stream.m3u");
    shout_set_password(_shout_stream,"dmr_decode");
    shout_open(_shout_stream);
}

ShoutStreamer::~ShoutStreamer()
{
    shout_close(_shout_stream);
    shout_free(_shout_stream);
    shout_shutdown();
}

int ShoutStreamer::work(int noutput_items, gr_vector_const_void_star &input_items, gr_vector_void_star &output_items)
{
    const unsigned char *in = (const unsigned char*)input_items[0];
    //unsigned char *out = (unsigned char*)output_items[0];

    shout_sync(_shout_stream);
    shout_send_raw(_shout_stream,in,noutput_items);
    return noutput_items;
}
