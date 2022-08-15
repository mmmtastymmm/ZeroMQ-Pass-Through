//
// Created by mmmtastymmm on 7/31/22.
//

#ifndef ZMQ_PASS_THROUGH_LOGGING_ABSTRACTION_H
#define ZMQ_PASS_THROUGH_LOGGING_ABSTRACTION_H
#include <boost/log/core.hpp>

class logging_abstraction
{
public:
    /// Sets the log level to the corresponding string. Will default to info if the string
    /// is invalid. \param log_level The level to set the boost singleton log level to.
    static void init_logging(const std::string& log_level);
};

#endif // ZMQ_PASS_THROUGH_LOGGING_ABSTRACTION_H
