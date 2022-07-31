#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <limits>
#include <utility>
#include <zmq.hpp>
#include <zmq_addon.hpp>

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

boost::program_options::options_description get_description()
{
    namespace po = boost::program_options;
    auto description = po::options_description("Allowed options");
    // clang-format off
    description.add_options()
            ("help", "produce help message")
            ("subscribe-address", po::value<std::string>()->default_value("127.0.0.1"), "Address to listen to.")
            ("subscribe-port", po::value<std::string>()->default_value("9090"), "Port to listen to.")
            ("subscribe-topic", po::value<std::string>()->default_value(""), "Subscribe topic to listen to.")
            ("publish-address", po::value<std::string>()->default_value("127.0.0.1"), "Address to publish to.")
            ("publish-port", po::value<std::string>()->default_value("9091"), "Port to publish to.")
            ("publish-topic", po::value<std::string>()->default_value(""), "Publish topic to publish. ")
            ("enable-publish", po::value<bool>()->default_value(true), "If the app should publish.")
            ("message-count", po::value<decltype(InputArgs::message_count)>()->default_value(std::numeric_limits<decltype(InputArgs::message_count)>::max()),"Messages to read before exit")
            ("log-level",  po::value<std::string>()->default_value("info"), "trace, debug, info, warning, error, or fatal should be used here.")
             ;
    // clang-format on
    return description;
}

InputArgs parse_input_args(int argc, char** argv)
{
    namespace po = boost::program_options;
    po::variables_map variables_map;
    const auto description = get_description();
    po::store(po::parse_command_line(argc, argv, description), variables_map);
    if (variables_map.count("help")) {
        std::cout << description << std::endl;
        std::exit(1);
    }
    return {variables_map.at("subscribe-address").as<std::string>(),
            variables_map.at("subscribe-port").as<std::string>(),
            variables_map.at("subscribe-topic").as<std::string>(),
            variables_map.at("publish-address").as<std::string>(),
            variables_map.at("publish-port").as<std::string>(),
            variables_map.at("publish-topic").as<std::string>(),
            variables_map.at("message-count").as<decltype(InputArgs::message_count)>(),
            variables_map.at("enable-publish").as<decltype(InputArgs::enable_publish)>(),
            variables_map.at("log-level").as<decltype(InputArgs::log_level)>()};
}

void init_logging(const std::string& log_level)
{
    if (log_level == "trace") {
        boost::log::core::get()->set_filter(boost::log::trivial::severity
                                            >= boost::log::trivial::trace);
    }
    else if (log_level == "debug") {
        boost::log::core::get()->set_filter(boost::log::trivial::severity
                                            >= boost::log::trivial::debug);
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
    }
}

int main(int argc, char** argv)
{
    auto input_args = parse_input_args(argc, argv);
    init_logging(input_args.log_level);
    zmq::context_t zmq_context_sub;
    zmq::socket_t subscriber(zmq_context_sub, zmq::socket_type::sub);
    auto listen_address = std::string("tcp://" + input_args.subscribe_address + ":"
                                      + input_args.subscribe_port);

    subscriber.connect(listen_address);
    subscriber.set(zmq::sockopt::subscribe, input_args.subscribe_topic);
    std::cout << "Subscribing to the following address: " << listen_address << std::endl;
    std::cout << "Will receive this many messages: " << input_args.message_count
              << std::endl;

    zmq::context_t zmq_context_pub;
    zmq::socket_t publisher(zmq_context_pub, zmq::socket_type::pub);
    auto publish_address = std::string("tcp://" + input_args.publish_address + ":"
                                       + input_args.publish_port);
    if (input_args.enable_publish){
        publisher.bind(publish_address);
        std::cout << "Now publishing to the following address: " << publish_address << std::endl;
    }
    else{
        std::cout << "Not publishing." << std::endl;
    }
    for (decltype(input_args.message_count) i = 0; i < input_args.message_count; i++) {
        auto receive_messages = std::vector<zmq::message_t>{};
        const auto ret =
            zmq::recv_multipart(subscriber, std::back_inserter(receive_messages));
        BOOST_LOG_TRIVIAL(info) << "Multipart message number: " << i;
        BOOST_LOG_TRIVIAL(info)
            << "Message count: " << receive_messages.size();
        if (receive_messages.size() > 1){
            BOOST_LOG_TRIVIAL(info) << "If first message was a topic: " << receive_messages.at(0).to_string();
        }

        auto all_messages = std::stringstream{};
        for (const auto& message : receive_messages) {
            all_messages << message << ", ";
        }
        BOOST_LOG_TRIVIAL(debug) << "All messages: " << all_messages.str();
        // Always send the topic if there is one
        if(not input_args.publish_topic.empty()) {
            publisher.send(zmq::message_t(input_args.publish_topic),
                           zmq::send_flags::sndmore);
        }
        // If there is only one message send that by itself, otherwise send everything but the previous topic
        if (receive_messages.size() == 1) {
            publisher.send(std::move(receive_messages.at(0)), zmq::send_flags::none);
        }
        else{
            for(auto j = receive_messages.begin() + 1; j != receive_messages.end() - 1; j++){
                publisher.send(*j,
                               zmq::send_flags::sndmore);
            }
            publisher.send(*(receive_messages.end() - 1),
                           zmq::send_flags::none);
        }
    }

    std::cout << "Done!" << std::endl;
    return 0;
}
