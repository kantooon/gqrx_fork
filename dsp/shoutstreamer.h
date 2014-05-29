#ifndef SHOUTSTREAMER_H
#define SHOUTSTREAMER_H

#include <gnuradio/sync_block.h>
#include <gnuradio/io_signature.h>
#include <stdio.h>
#include <shout/shout.h>

class ShoutStreamer;

typedef boost::shared_ptr<ShoutStreamer> shoutstreamer_sptr;

shoutstreamer_sptr make_shoutstreamer();

class ShoutStreamer: public gr::sync_block
{
public:
    ShoutStreamer();
    ~ShoutStreamer();
    int work(int noutput_items,
           gr_vector_const_void_star &input_items,
           gr_vector_void_star &output_items);

private:
    shout_t *_shout_stream;
};

#endif // SHOUTSTREAMER_H
