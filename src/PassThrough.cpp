#include "PassThrough.h"
#include <boost/log/core.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/trivial.hpp>
#include <boost/program_options.hpp>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/information.hpp>
#include <iostream>
#include <limits>
#include <utility>
#include <zmq.hpp>
#include <zmq_addon.hpp>
#include "logging_abstraction.h"

boost::program_options::options_description PassThrough::get_description()
{
    namespace po = boost::program_options;
    auto description = po::options_description("Allowed options");
    // clang-format off
    description.add_options()
            ("help,h", "produce help message")
            ("subscribe-address", po::value<decltype(PassThrough::InputArgs::subscribe_address)>()->default_value("0.0.0.0"), "Address to listen to.")
            ("subscribe-port", po::value<decltype(PassThrough::InputArgs::subscribe_port)>()->default_value("9090"), "Port to listen to.")
            ("subscribe-topic", po::value<decltype(PassThrough::InputArgs::subscribe_topic)>()->default_value(""), "Subscribe topic to listen to.")
            ("publish-address", po::value<decltype(PassThrough::InputArgs::publish_address)>()->default_value("0.0.0.0"), "Address to publish to.")
            ("publish-port", po::value<decltype(PassThrough::InputArgs::publish_port)>()->default_value("9091"), "Port to publish to.")
            ("publish-topic", po::value<decltype(PassThrough::InputArgs::publish_topic)>()->default_value(""), "Publish topic to publish. ")
            ("enable-publish", po::value<decltype(PassThrough::InputArgs::enable_publish)>()->default_value(true), "If the app should publish.")
            ("message-count", po::value<decltype(PassThrough::InputArgs::message_count)>()->default_value(std::numeric_limits<decltype(PassThrough::InputArgs::message_count)>::max()),"Messages to read before exit")
            ("log-level",  po::value<decltype(PassThrough::InputArgs::log_level)>()->default_value("info"), "trace, debug, info, warning, error, or fatal should be used here.")
             ;
    // clang-format on
    return description;
}

PassThrough::InputArgs PassThrough::parse_input_args(int argc, char** argv)
{
    namespace po = boost::program_options;
    po::variables_map variables_map;
    const auto description = PassThrough::get_description();
    po::store(po::parse_command_line(argc, argv, description), variables_map);
    if (variables_map.count("help")) {
        std::cout << description << std::endl;
        std::exit(1);
    }
    return {
        variables_map.at("subscribe-address")
            .as<decltype(PassThrough::InputArgs::subscribe_address)>(),
        variables_map.at("subscribe-port")
            .as<decltype(PassThrough::InputArgs::subscribe_port)>(),
        variables_map.at("subscribe-topic")
            .as<decltype(PassThrough::InputArgs::subscribe_topic)>(),
        variables_map.at("publish-address")
            .as<decltype(PassThrough::InputArgs::publish_address)>(),
        variables_map.at("publish-port")
            .as<decltype(PassThrough::InputArgs::publish_port)>(),
        variables_map.at("publish-topic")
            .as<decltype(PassThrough::InputArgs::publish_topic)>(),
        variables_map.at("message-count")
            .as<decltype(PassThrough::InputArgs::message_count)>(),
        variables_map.at("enable-publish")
            .as<decltype(PassThrough::InputArgs::enable_publish)>(),
        variables_map.at("log-level").as<decltype(PassThrough::InputArgs::log_level)>()};
}

int PassThrough::main(int argc, char** argv)
{
    // Set up the subscriber
    auto input_args = PassThrough::parse_input_args(argc, argv);
    logging_abstraction::init_logging(input_args.log_level);
    zmq::context_t zmq_context_sub;
    zmq::socket_t subscriber(zmq_context_sub, zmq::socket_type::sub);
    auto listen_address = std::string("tcp://" + input_args.subscribe_address + ":"
                                      + input_args.subscribe_port);

    std::cout << "Subscribing to the following address: " << listen_address << std::endl;
    subscriber.connect(listen_address);
    subscriber.set(zmq::sockopt::subscribe, input_args.subscribe_topic);
    std::cout << "Will receive this many messages: " << input_args.message_count
              << std::endl;

    // Set up the publisher
    zmq::context_t zmq_context_pub;
    zmq::socket_t publisher(zmq_context_pub, zmq::socket_type::pub);
    auto publish_address = std::string("tcp://" + input_args.publish_address + ":"
                                       + input_args.publish_port);
    if (input_args.enable_publish) {
        std::cout << "Publishing to the following address: " << publish_address
                  << std::endl;
        publisher.bind(publish_address);
    }
    else {
        std::cout << "Not publishing." << std::endl;
    }

    // Receive the requested number of messages and terminate
    std::unordered_map<std::string, PassThrough::DataResults> data_by_topic;
    for (decltype(input_args.message_count) i = 0; i < input_args.message_count; i++) {
        process_message(input_args, subscriber, publisher, i, data_by_topic);
    }
    // Print out the stats for each topic
    for (auto& topic : data_by_topic) {
        std::cout << "Topic: " << topic.first << ", Data: " << topic.second << std::endl;
    }
    std::cout << "Done!" << std::endl;
    return 0;
}

bool PassThrough::process_message(const InputArgs& input_args,
                                  zmq::socket_t& subscriber,
                                  zmq::socket_t& publisher,
                                  long long int i,
                                  std::unordered_map<std::string, DataResults>& map)
{
    auto receive_messages = std::vector<zmq::message_t>{};
    const auto ret =
        zmq::recv_multipart(subscriber, std::back_inserter(receive_messages));
    if (not ret) {
        BOOST_LOG_TRIVIAL(warning) << "Encountered a corrupted message on iteration " << i
                                   << ". Will ignore and continue.";
        return false;
    }
    std::stringstream info_log;
    info_log << "Multipart message number: " << i
             << ", Message count: " << receive_messages.size() << ", ";
    auto topic = std::string("");
    if (receive_messages.size() > 1) {
        info_log << "If first message was a topic: " << receive_messages.at(0).to_string()
                 << ", ";
        topic = receive_messages.at(0).to_string();
    }
    auto total_message_size = decltype(receive_messages.at(0).size())(0);
    for (const auto& message : receive_messages) {
        total_message_size += message.size();
    }
    info_log << "Total total_message_size: " << total_message_size;
    BOOST_LOG_TRIVIAL(info) << info_log.str();
    // Update the topic map for the whole run
    map.try_emplace(topic, PassThrough::DataResults());
    map.at(topic).update(total_message_size);
    auto all_messages = std::stringstream{};
    for (const auto& message : receive_messages) {
        all_messages << message << ", ";
    }
    BOOST_LOG_TRIVIAL(debug) << "All messages: " << all_messages.str();
    // Always send the topic if there is one
    if (not input_args.publish_topic.empty()) {
        publisher.send(zmq::message_t(input_args.publish_topic),
                       zmq::send_flags::sndmore);
    }
    // If there is only one message send that by itself, otherwise send everything but
    // the previous topic
    if (receive_messages.size() == 1) {
        publisher.send(std::move(receive_messages.at(0)), zmq::send_flags::none);
    }
    else {
        for (auto j = receive_messages.begin() + 1; j != receive_messages.end() - 1;
             j++) {
            publisher.send(*j, zmq::send_flags::sndmore);
        }
        publisher.send(*(receive_messages.end() - 1), zmq::send_flags::none);
    }
    return true;
}
void PassThrough::DataResults::update(size_t new_message_size)
{
    message_count += 1;
    total_bytes +=
        static_cast<double>(new_message_size) * boost::units::information::bytes;
    if (message_count == 1) {
        average_size =
            static_cast<double>(new_message_size) * boost::units::information::bytes;
    }
    else {
        average_size +=
            1.0 / static_cast<double>(message_count)
            * (static_cast<double>(new_message_size) * boost::units::information::bytes
               - average_size);
    }
}
std::ostream& operator<<(std::ostream& os, const PassThrough::DataResults& results)
{
    os << "message_count: " << results.message_count << boost::units::binary_prefix
       << ", total_bytes: " << results.total_bytes
       << ", average_size: " << results.average_size << boost::units::symbol_format;
    return os;
}
