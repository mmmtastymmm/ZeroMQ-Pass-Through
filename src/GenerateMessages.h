//
// Created by mmmtastymmm on 7/31/22.
//

#ifndef ZMQ_PASS_THROUGH_GENERATEMESSAGES_H
#define ZMQ_PASS_THROUGH_GENERATEMESSAGES_H
#include <string>

class GenerateMessages
{
public:
    struct InputArgs
    {
        std::string address;
        std::string port;
        long long message_count;
        std::string message_text;
        bool append_counter{};

        InputArgs(std::string address,
                  std::string port,
                  long long int messageCount,
                  std::string messageText,
                  bool appendCounter);
    };
    static int main(int argc, char** argv);
};

#endif // ZMQ_PASS_THROUGH_GENERATEMESSAGES_H
