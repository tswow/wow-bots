#pragma once

#include "packet-builder.h"

PacketBuilder* CMSGSetActiveMover = PacketBuilder
    ::CreatePacket(Opcodes::CMSG_SET_ACTIVE_MOVER, PacketType::WRITE, "CMSGSetActiveMover")
    ->SingleField("uint64","GUID")
    ;