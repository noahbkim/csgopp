#pragma once

#include <chrono>
#include <iostream>

#include <google/protobuf/io/coded_stream.h>

using google::protobuf::io::CodedInputStream;
using google::protobuf::io::ZeroCopyInputStream;
using google::protobuf::io::IstreamInputStream;

struct Timer
{
    std::chrono::time_point<std::chrono::system_clock> origin = std::chrono::system_clock::now();
    std::chrono::time_point<std::chrono::system_clock> last = origin;
};

std::ostream& operator<<(std::ostream& out, Timer& timer)
{
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now() - timer.last).count();
    timer.last = std::chrono::system_clock::now();
    return out << elapsed << " ms";
}
