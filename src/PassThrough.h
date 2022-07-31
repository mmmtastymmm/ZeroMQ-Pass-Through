#ifndef ZMQ_PASS_THROUGH_PASSTHROUGH_H
#define ZMQ_PASS_THROUGH_PASSTHROUGH_H
#include <string>
#include <boost/program_options/options_description.hpp>

class PassThrough
{
public:
    struct InputArgs
    {
        std::string subscribe_address;
        std::string subscribe_port;
        std::string subscribe_topic;
        std::string publish_address;
        std::string publish_port;
        std::string publish_topic;
        long long message_count;
        bool enable_publish;
        std::string log_level;

        InputArgs(std::string subscribeAddress,
                  std::string subscribePort,
                  std::string subscribeTopic,
                  std::string publishAddress,
                  std::string publishPort,
                  std::string publishTopic,
                  decltype(message_count) messageCount,
                  bool shouldPublish,
                  std::string logLevel)
            : subscribe_address(std::move(subscribeAddress)),
              subscribe_port(std::move(subscribePort)),
              subscribe_topic(std::move(subscribeTopic)),
              publish_address(std::move(publishAddress)),
              publish_port(std::move(publishPort)),
              publish_topic(std::move(publishTopic)),
              message_count(messageCount),
              enable_publish(shouldPublish),
              log_level(std::move(logLevel))
        {
        }
    };
    static int main(int argc, char** argv);
    static boost::program_options::options_description get_description();
    static PassThrough::InputArgs parse_input_args(int argc, char** argv);
};

#endif // ZMQ_PASS_THROUGH_PASSTHROUGH_H
