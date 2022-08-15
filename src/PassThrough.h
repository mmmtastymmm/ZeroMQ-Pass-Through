#ifndef ZMQ_PASS_THROUGH_PASSTHROUGH_H
#define ZMQ_PASS_THROUGH_PASSTHROUGH_H
#include <string>
#include <boost/program_options/options_description.hpp>
#include <boost/units/quantity.hpp>
#include <boost/units/io.hpp>
#include <boost/units/systems/information.hpp>
#include <zmq.hpp>

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

    struct DataResults
    {
        // a quantity of information (default in units of total_bytes)
        boost::units::quantity<boost::units::information::info> total_bytes{
            0.0 * boost::units::information::byte};
        size_t message_count{0};
        decltype(total_bytes) average_size{0.0 * boost::units::information::byte};
        void update(size_t new_message_size);
    };

    /// Runs the program given the command line input
    /// \param argc the number of command line args passed
    /// \param argv the values of the command line args passed
    /// \return 0 if successful, something else if an error occurred.
    static int main(int argc, char** argv);

    /// Gets the boost program description (what command line args are accepted)
    /// \return The boost program description
    static boost::program_options::options_description get_description();

    /// Parse the input args and return the input args class
    /// \param argc the number of command line args passed
    /// \param argv the values of the command line args passed
    /// \return An InputArgs instance, populated from the command line args and defaults
    /// if required
    static PassThrough::InputArgs parse_input_args(int argc, char** argv);

    /// Processes a message
    /// \param input_args the input args
    /// \param subscriber The subscriber socket
    /// \param publisher The publisher socket
    /// \param i The iteration
    /// \return true if the message was received in a healthy state, false if not
    static bool process_message(const InputArgs& input_args,
                                zmq::socket_t& subscriber,
                                zmq::socket_t& publisher,
                                long long int i);
};

#endif // ZMQ_PASS_THROUGH_PASSTHROUGH_H
