//
// Created by jorge on 11/27/2023.
//

#ifndef SPHERO_ROBOT_NETWORK_HELPER_HPP
#define SPHERO_ROBOT_NETWORK_HELPER_HPP
// This code has been taken from testable_networking made by markaren and will be modified to fit our needs if necessary.
#include <array>

enum class byte_order {
    LITTLE, BIG
};

std::array<unsigned char, 4> int_to_bytes(int n, byte_order order = byte_order::LITTLE) {
    std::array<unsigned char, 4> bytes{};

    if (order == byte_order::LITTLE) {
        bytes[0] = n & 0xFF;
        bytes[1] = (n >> 8) & 0xFF;
        bytes[2] = (n >> 16) & 0xFF;
        bytes[3] = (n >> 24) & 0xFF;
    } else {
        bytes[0] = (n >> 24) & 0xFF;
        bytes[1] = (n >> 16) & 0xFF;
        bytes[2] = (n >> 8) & 0xFF;
        bytes[3] = n & 0xFF;
    }

    return bytes;
}

int bytes_to_int(std::array<unsigned char, 4> buffer, byte_order order = byte_order::LITTLE) {
    if (order == byte_order::LITTLE) {
        return int(buffer[0] |
                   buffer[1] << 8 |
                   buffer[2] << 16 |
                   buffer[3] << 24);
    } else {
        return int(buffer[0] << 24 |
                   buffer[1] << 16 |
                   buffer[2] << 8 |
                   buffer[3]);
    }
}



#endif//SPHERO_ROBOT_NETWORK_HELPER_HPP
