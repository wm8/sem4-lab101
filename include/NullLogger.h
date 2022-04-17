//Copyright 2022 wm8
#ifndef LAB10_NULLLOGGER_H
#define LAB10_NULLLOGGER_H
#define MESSAGE_LOG(lvl)\
    BOOST_LOG_STREAM_WITH_PARAMS(::boost::log::trivial::logger::get(),\
        (::boost::log::keywords::severity = lvl))
#endif  // LAB10_NULLLOGGER_H
