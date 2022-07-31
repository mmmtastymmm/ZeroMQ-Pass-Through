#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>

#include "logging_abstraction.h"
void logging_abstraction::init_logging(const std::string& log_level)
{
    {
        if (log_level == "trace") {
            boost::log::core::get()->set_filter(boost::log::trivial::severity
                                                >= boost::log::trivial::trace);
        }
        else if (log_level == "debug") {
            boost::log::core::get()->set_filter(boost::log::trivial::severity
                                                >= boost::log::trivial::debug);
        }
        else if (log_level == "info") {
            boost::log::core::get()->set_filter(boost::log::trivial::severity
                                                >= boost::log::trivial::info);
        }
        else if (log_level == "warning") {
            boost::log::core::get()->set_filter(boost::log::trivial::severity
                                                >= boost::log::trivial::warning);
        }
        else if (log_level == "error") {
            boost::log::core::get()->set_filter(boost::log::trivial::severity
                                                >= boost::log::trivial::error);
        }
        else if (log_level == "fatal") {
            boost::log::core::get()->set_filter(boost::log::trivial::severity
                                                >= boost::log::trivial::fatal);
        }
        else {
            boost::log::core::get()->set_filter(boost::log::trivial::severity
                                                >= boost::log::trivial::info);
            BOOST_LOG_TRIVIAL(warning)
                << "Log level wasn't properly specified so setting log level to "
                   "\"info\". Incorrectly specified log level: "
                << log_level;
        }
    }
}
