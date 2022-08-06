#include <iostream>

#include "packet-builder.h"

std::vector<std::unique_ptr<PacketBuilder>>& PacketBuilder::GetBuilders()
{
    static std::vector<std::unique_ptr<PacketBuilder>> packets;
    return packets;
}

int main() {
    // Include your packet definitions here
    #include "Login.h"

    // Do not include packet definitions after this call
    PacketBuilder::Generate();
}