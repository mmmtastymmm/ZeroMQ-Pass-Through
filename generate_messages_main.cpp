#include <iostream>
#include <boost/program_options.hpp>
#include <thread>
#include <zmq.hpp>

boost::program_options::options_description get_description() {
    namespace po = boost::program_options;
    auto description = po::options_description("Allowed options");
    description.add_options()
            ("help", "produce help message")
            ("ip", po::value<std::string>()->default_value("127.0.0.1"), "Ip to listen to")
            ("port", po::value<std::string>()->default_value("9090"), "Port to listen to")
            ("message-count", po::value<long long>()->default_value(500),
             "Messages to send before exit");
    return description;
}

struct InputArgs {
    std::string ip;
    std::string port;
    long long message_count;

    InputArgs(std::string ip, std::string port, long long message_count) : ip(std::move(ip)), port(std::move(port)),
                                                                     message_count(message_count) {}
};

InputArgs parse_input_args(int argc, char **argv) {
    namespace po = boost::program_options;
    po::variables_map variables_map;
    const auto description = get_description();
    po::store(po::parse_command_line(argc, argv, description), variables_map);
    if (variables_map.count("help")) {
        std::cout << description << std::endl;
        std::exit(1);
    }
    return {variables_map.at("ip").as<std::string>(), variables_map.at("port").as<std::string>(), variables_map.at("message-count").as<long long>()};
}

int main(int argc, char** argv) {
    auto input_args = parse_input_args(argc, argv);
    zmq::context_t zmq_context;
    zmq::socket_t socket(zmq_context, zmq::socket_type::pub);

    auto address = "tcp://" + input_args.ip + ":" + input_args.port;
    std::cout << "Now listening on: " << address << std::endl;
    socket.bind( address);
    std::cout << "Will send this many messages: " << input_args.message_count << std::endl;
    for (int i = 0; i < input_args.message_count; i++){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        auto message = zmq::message_t(std::string("Hello world " + std::to_string(i + 1)));
        socket.send(message, zmq::send_flags::none);
        std::cout << "Sent message " << i << std::endl;
    }

    std::cout << "Done!" << std::endl;
    return 0;
}
