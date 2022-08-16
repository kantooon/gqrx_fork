#--------------------------------------------------------------------------------
#
# Qmake project file for gqrx - http://gqrx.dk
#
# Common options you may want to passs to qmake:
#
#    CONFIG+=debug            Enable debug mode
#    PREFIX=/some/prefix      Installation prefix
#    BOOST_SUFFIX=-mt         To link against libboost-xyz-mt (needed for pybombs)
#    AUDIO_BACKEND=portaudio  Use it on Mac OS X to have FCD Pro and Pro+ support
#--------------------------------------------------------------------------------

QT       += core gui svg
contains(QT_MAJOR_VERSION,5) {
    QT += widgets
}

TEMPLATE = app

macx {
    TARGET = Gqrx
    ICON = icons/scope.icns
    DEFINES += GQRX_OS_MACX
} else {
    TARGET = gqrx
}

unix:!macx {
    CONFIG += link_pkgconfig
    packagesExist(libpulse libpulse-simple) {
        # Comment out to use gr-audio (not recommended with ALSA and Funcube Dongle Pro)
        AUDIO_BACKEND = pulse
        message("Gqrx configured with pulseaudio backend")
    }
}

RESOURCES += icons.qrc

# make clean target
QMAKE_CLEAN += gqrx

# make install target
isEmpty(PREFIX) {
    message(No prefix given. Using /usr/local)
    PREFIX=/usr/local
}

target.path  = $$PREFIX/bin
INSTALLS    += target 

#CONFIG += debug

# disable debug messages in release
CONFIG(debug, debug|release) {
    # Use for valgrind
    #QMAKE_CFLAGS_DEBUG += '-g -O0'

    # Define version string (see below for releases)
    VER = $$system(git describe --abbrev=8)

} else {
    DEFINES += QT_NO_DEBUG
    DEFINES += QT_NO_DEBUG_OUTPUT
    VER = $$system(git describe --abbrev=1)

    # Release binaries with gr bundled
    # QMAKE_RPATH & co won't work with origin
    ## QMAKE_LFLAGS += '-Wl,-rpath,\'\$$ORIGIN/lib\''
}


# Tip from: http://www.qtcentre.org/wiki/index.php?title=Version_numbering_using_QMake
VERSTR = '\\"$${VER}\\"'          # place quotes around the version string
DEFINES += VERSION=\"$${VERSTR}\" # create a VERSION macro containing the version string

SOURCES += \
    applications/gqrx/main.cpp \
    applications/gqrx/mainwindow.cpp \
    applications/gqrx/receiver.cpp \
    dsp/afsk1200/cafsk12.cpp \
    dsp/afsk1200/costabf.c \
    dsp/agc_impl.cpp \
    dsp/correct_iq_cc.cpp \
    dsp/lpf.cpp \
    dsp/resampler_xx.cpp \
    dsp/rx_demod_am.cpp \
    dsp/rx_demod_fm.cpp \
    dsp/rx_fft.cpp \
    dsp/rx_filter.cpp \
    dsp/rx_meter.cpp \
    dsp/rx_agc_xx.cpp \
    dsp/rx_noise_blanker_cc.cpp \
    dsp/sniffer_f.cpp \
    dsp/stereo_demod.cpp \
    qtgui/afsk1200win.cpp \
    qtgui/agc_options.cpp \
    qtgui/audio_options.cpp \
    qtgui/demod_options.cpp \
    qtgui/dockinputctl.cpp \
    qtgui/dockaudio.cpp \
    qtgui/dockfft.cpp \
    qtgui/dockiqplayer.cpp \
    qtgui/dockrxopt.cpp \
    qtgui/freqctrl.cpp \
    qtgui/gain_options.cpp \
    qtgui/ioconfig.cpp \
    qtgui/meter.cpp \
    qtgui/nb_options.cpp \
    qtgui/plotter.cpp \
    qtgui/qtcolorpicker.cpp \
    receivers/nbrx.cpp \
    receivers/receiver_base.cpp \
    receivers/wfmrx.cpp \
    dsp/rx_demod_qpsk.cpp \
    dsp/qpsk_to_audio.cpp \
    dsp/shoutstreamer.cpp

HEADERS += \
    applications/gqrx/mainwindow.h \
    applications/gqrx/receiver.h \
    applications/gqrx/gqrx.h \
    dsp/afsk1200/cafsk12.h \
    dsp/afsk1200/filter.h \
    dsp/afsk1200/filter-i386.h \
    dsp/agc_impl.h \
    dsp/correct_iq_cc.h \
    dsp/lpf.h \
    dsp/resampler_xx.h \
    dsp/rx_agc_xx.h \
    dsp/rx_demod_am.h \
    dsp/rx_demod_fm.h \
    dsp/rx_fft.h \
    dsp/rx_filter.h \
    dsp/rx_meter.h \
    dsp/rx_noise_blanker_cc.h \
    dsp/sniffer_f.h \
    dsp/stereo_demod.h \
    qtgui/afsk1200win.h \
    qtgui/agc_options.h \
    qtgui/audio_options.h \
    qtgui/demod_options.h \
    qtgui/dockaudio.h \
    qtgui/dockfft.h \
    qtgui/dockinputctl.h \
    qtgui/dockiqplayer.h \
    qtgui/dockrxopt.h \
    qtgui/freqctrl.h \
    qtgui/gain_options.h \
    qtgui/ioconfig.h \
    qtgui/meter.h \
    qtgui/nb_options.h \
    qtgui/plotter.h \
    qtgui/qtcolorpicker.h \
    receivers/nbrx.h \
    receivers/receiver_base.h \
    receivers/wfmrx.h \
    dsp/rx_demod_qpsk.h \
    dsp/qpsk_to_audio.h \
    dsp/shoutstreamer.h

FORMS += \
    applications/gqrx/mainwindow.ui \
    qtgui/afsk1200win.ui \
    qtgui/agc_options.ui \
    qtgui/audio_options.ui \
    qtgui/demod_options.ui \
    qtgui/dockaudio.ui \
    qtgui/dockfft.ui \
    qtgui/dockiqplayer.ui \
    qtgui/dockinputctl.ui \
    qtgui/dockrxopt.ui \
    qtgui/gain_options.ui \
    qtgui/ioconfig.ui \
    qtgui/nb_options.ui

# Use pulseaudio (ps: could use equals? undocumented)
contains(AUDIO_BACKEND, pulse): {
    HEADERS += \
        pulseaudio/pa_device_list.h \
        pulseaudio/pa_sink.h \
        pulseaudio/pa_source.h
    SOURCES += \
        pulseaudio/pa_device_list.cc \
        pulseaudio/pa_sink.cc \
        pulseaudio/pa_source.cc
    DEFINES += WITH_PULSEAUDIO
}

# Introduced in 2.2 for FCD support on OS X
contains(AUDIO_BACKEND, portaudio): {
    HEADERS += portaudio/device_list.h
    SOURCES += portaudio/device_list.cpp
    DEFINES += WITH_PORTAUDIO
}

# dependencies via pkg-config
# FIXME: check for version?
unix:!macx {
    contains(AUDIO_BACKEND, pulse): {
        PKGCONFIG += libpulse libpulse-simple
    } else {
        PKGCONFIG += gnuradio-audio
    }
    PKGCONFIG += gnuradio-analog \
                 gnuradio-blocks \
                 gnuradio-filter \
                 gnuradio-fft \
                 gnuradio-osmosdr \
                 gnuradio-digital

    LIBS += -lboost_system$$BOOST_SUFFIX -lboost_program_options$$BOOST_SUFFIX
    LIBS += -lrt  # need to include on some distros
    LIBS += -lgr-dsd
    LIBS += -losmocore
    LIBS += -lshout
    DEPENDPATH += $$PWD/../../c++/osmo-tetra/src
    LIBS += -L$$PWD/../../c++/osmo-tetra/src -losmo-tetra-phy -L$$PWD/../../c++/osmo-tetra/src -losmo-tetra-mac
    INCLUDEPATH += $$PWD/../../c++/osmo-tetra/src

}

macx {
    # macports
    INCLUDEPATH += /opt/local/include

    # local stuff
    INCLUDEPATH += /Users/alexc/gqrx/runtime/include
    LIBS += -L/opt/local/lib -L/Users/alexc/gqrx/runtime/lib

    LIBS += -lboost_system-mt -lboost_program_options-mt
    LIBS += -lgnuradio-runtime -lgnuradio-pmt -lgnuradio-audio -lgnuradio-analog
    LIBS += -lgnuradio-blocks -lgnuradio-filter -lgnuradio-fft -lgnuradio-osmosdr

    # portaudio
    contains(AUDIO_BACKEND, portaudio): {
        LIBS    += -lportaudio
    }
}

OTHER_FILES += \
    README.md \
    COPYING \
    news.txt

