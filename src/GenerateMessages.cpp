#include <iostream>
#include <boost/program_options.hpp>
#include <thread>
#include <utility>
#include <zmq.hpp>
#include "GenerateMessages.h"

boost::program_options::options_description get_description()
{
    namespace po = boost::program_options;
    auto description = po::options_description("Allowed options");
    description.add_options()
        // clang-format off
            ("help,h", "produce help message")
            ("address", po::value<decltype(GenerateMessages::InputArgs::address)>()->default_value("0.0.0.0"), "Address to publish to")
            ("port", po::value<decltype(GenerateMessages::InputArgs::port)>()->default_value("9090"), "Port to publish to")
            ("message-count", po::value<decltype(GenerateMessages::InputArgs::message_count)>()->default_value(500), "How many messages to send before exit")
            ("message-text", po::value<decltype(GenerateMessages::InputArgs::message_text)>()->default_value("Hello world"), "What text to send on the message")
            ("append-counter", po::value<decltype(GenerateMessages::InputArgs::append_counter)>()->default_value(true), "If a ascending unique id should be appended to the message text")
            ;
    // clang-format on
    return description;
}

GenerateMessages::InputArgs parse_input_args(int argc, char** argv)
{
    namespace po = boost::program_options;
    po::variables_map variables_map;
    const auto description = get_description();
    po::store(po::parse_command_line(argc, argv, description), variables_map);
    if (variables_map.count("help")) {
        std::cout << description << std::endl;
        std::exit(1);
    }
    return {
        variables_map.at("address").as<decltype(GenerateMessages::InputArgs::address)>(),
        variables_map.at("port").as<decltype(GenerateMessages::InputArgs::port)>(),
        variables_map.at("message-count")
            .as<decltype(GenerateMessages::InputArgs::message_count)>(),
        variables_map.at("message-text")
            .as<decltype(GenerateMessages::InputArgs::message_text)>(),
        variables_map.at("append-counter")
            .as<decltype(GenerateMessages::InputArgs::append_counter)>(),
    };
}

int GenerateMessages::main(int argc, char** argv)
{
    auto input_args = parse_input_args(argc, argv);
    zmq::context_t zmq_context;
    zmq::socket_t socket(zmq_context, zmq::socket_type::pub);

    auto address = "tcp://" + input_args.address + ":" + input_args.port;
    std::cout << "Now listening on: " << address << std::endl;
    socket.bind(address);
    std::cout << "Will send this many messages: " << input_args.message_count
              << std::endl;
    for (int i = 0; i < input_args.message_count; i++) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        auto message =
            zmq::message_t(std::string("Hello world " + std::to_string(i + 1)));
        socket.send(message, zmq::send_flags::none);
        std::cout << "Sent message " << i << std::endl;
    }

    std::cout << "Done!" << std::endl;
    return 0;
}
GenerateMessages::InputArgs::InputArgs(std::string address,
                                       std::string port,
                                       long long int messageCount,
                                       std::string messageText,
                                       bool appendCounter)
    : address(std::move(address)),
      port(std::move(port)),
      message_count(messageCount),
      message_text(std::move(messageText)),
      append_counter(appendCounter)
{
}
