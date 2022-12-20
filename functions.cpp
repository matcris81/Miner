#include "functions.h"

std::string bytesToHex(std::vector<uint8_t> bytes)
{
    std::stringstream ss;
    ss << std::hex;
    for (int i(0); i < bytes.size(); ++i)
        ss << std::setw(2) << std::setfill('0') << (int)bytes[i];
    return ss.str();
}