#include <catch2/catch_test_macros.hpp>
#include "../src/PassThrough.h"

TEST_CASE("Test data results constructor", "[DataResults]")
{
    auto empty = PassThrough::DataResults();
    CHECK(empty.message_count == 0);
    CHECK(empty.average_size == 0.0 * boost::units::information::byte);
    CHECK(empty.total_bytes == 0.0 * boost::units::information::byte);
}

TEST_CASE("Test data after 1 append", "[DataResults]")
{
    auto data = PassThrough::DataResults();
    auto size = 100;
    auto size_double = static_cast<double>(size);
    data.update(size);
    CHECK(data.message_count == 1);
    CHECK(data.average_size == size_double * boost::units::information::byte);
    CHECK(data.total_bytes == size_double * boost::units::information::byte);
}

TEST_CASE("Test data after multiple appends", "[DataResults]")
{
    auto data = PassThrough::DataResults();
    auto sizes = std::vector<size_t>{100, 50};
    for (auto size : sizes) {
        data.update(size);
    }

    CHECK(data.message_count == 2);
    CHECK(data.average_size == 75.0 * boost::units::information::byte);
    CHECK(data.total_bytes == 150.0 * boost::units::information::byte);
}