#include "Movement.h"
#include "BotPacket.h"
#include "Bot.h"

#include <sol/sol.hpp>
#include <chrono>

static uint64 now()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}
// todo: udb
uint64 start = now();

MovementPacket MovementPacket::create(Opcodes opcode)
{
    MovementPacket packet;
    packet.time = uint32(now() - start);
    packet.opcode = opcode;
    return packet;
}


Opcodes MovementPacket::GetOpcode()
{
    return opcode;
}

MovementPacket& MovementPacket::SetOpcode(Opcodes _opcode)
{
    opcode = _opcode;
    return *this;
}

MovementPacket MovementPacket::Read(WorldPacket& packet)
{
    MovementPacket movement;
    movement.GUID = packet.ReadPackedGUID();
    movement.opcode = packet.GetOpcode();
    movement.flags = packet.ReadUInt32();
    movement.flags2 = packet.ReadUInt16();
    movement.time = packet.ReadUInt32();
    movement.x = packet.ReadFloat();
    movement.y = packet.ReadFloat();
    movement.z = packet.ReadFloat();
    movement.o = packet.ReadFloat();

    if (movement.flags & MOVEMENTFLAG_ONTRANSPORT)
    {
        movement.GUID = packet.ReadUInt64();
        movement.transportX = packet.ReadFloat();
        movement.transportY = packet.ReadFloat();
        movement.transportZ = packet.ReadFloat();
        movement.transportO = packet.ReadFloat();
        movement.transportTime = packet.ReadUInt32();
        movement.transportSeat = packet.ReadInt8();

        if (movement.flags2 & MOVEMENTFLAG2_INTERPOLATED_MOVEMENT)
        {
            movement.transportTime2 = packet.ReadUInt32();
        }
    }

    if ((movement.flags | MOVEMENTFLAG_SWIMMING) || (movement.flags | MOVEMENTFLAG_FLYING) || movement.flags2 | MOVEMENTFLAG2_ALWAYS_ALLOW_PITCHING)
    {
        movement.pitch = packet.ReadFloat();
    }

    movement.fallTime = packet.ReadUInt32();

    if (movement.flags & MOVEMENTFLAG_FALLING)
    {
        movement.jumpZSpeed = packet.ReadFloat();
        movement.jumpSinAngle = packet.ReadFloat();
        movement.jumpCosAngle = packet.ReadFloat();
        movement.jumpXYSpeed = packet.ReadFloat();
    }

    if (movement.flags & MOVEMENTFLAG_SPLINE_ELEVATION)
    {
        movement.splineElevation = packet.ReadFloat();
    }

    return movement;
}

WorldPacket MovementPacket::Write()
{
    WorldPacket packet(opcode);
    packet.WritePackedGUID(GUID);
    packet.WriteUInt32(flags);
    packet.WriteUInt16(flags2);
    packet.WriteUInt32(time);
    packet.WriteFloat(x);
    packet.WriteFloat(y);
    packet.WriteFloat(z);
    packet.WriteFloat(o);
    if (flags & MOVEMENTFLAG_ONTRANSPORT)
    {
        packet.WriteFloat(transportX);
        packet.WriteFloat(transportY);
        packet.WriteFloat(transportZ);
        packet.WriteFloat(transportO);
        packet.WriteUInt32(transportTime);
        packet.WriteInt8(transportSeat);
        if (flags2 & MOVEMENTFLAG2_INTERPOLATED_MOVEMENT)
        {
            packet.WriteUInt32(transportTime2);
        }
    }

    if ((flags | MOVEMENTFLAG_SWIMMING) || (flags | MOVEMENTFLAG_FLYING) || flags2 | MOVEMENTFLAG2_ALWAYS_ALLOW_PITCHING)
    {
        packet.WriteFloat(pitch);
    }

    packet.WriteUInt32(fallTime);

    if (flags & MOVEMENTFLAG_FALLING)
    {
        packet.WriteFloat(jumpZSpeed);
        packet.WriteFloat(jumpSinAngle);
        packet.WriteFloat(jumpCosAngle);
        packet.WriteFloat(jumpXYSpeed);
    }

    if (flags & MOVEMENTFLAG_SPLINE_ELEVATION)
    {
        packet.WriteFloat(splineElevation);
    }

    return packet;
}

void MovementPacket::Send(Bot& bot)
{
    Write().SendNoWait(bot);
}

uint64 MovementPacket::GetGUID()
{
    return GUID;
}

uint32 MovementPacket::GetFlags()
{
    return flags;
}

uint16 MovementPacket::GetFlags2()
{
    return flags2;
}

float MovementPacket::GetX()
{
    return x;
}
float MovementPacket::GetY()
{
    return y;
}
float MovementPacket::GetZ()
{
    return z;
}
float MovementPacket::GetO()
{
    return o;
}
uint32 MovementPacket::GetTime()
{
    return time;
}

uint64 MovementPacket::GetTransportGUID()
{
    return TransportGUID;
}
float MovementPacket::GetTransportX()
{
    return transportX;
}
float MovementPacket::GetTransportY()
{
    return transportY;
}
float MovementPacket::GetTransportZ()
{
    return transportZ;
}
float MovementPacket::GetTransportO()
{
    return transportO;
}

int8 MovementPacket::GetTransportSeat()
{
    return transportSeat;
}
uint32 MovementPacket::GetTransportTime()
{
    return transportTime;
}
uint32 MovementPacket::GetTransportTime2()
{
    return transportTime2;
}

float MovementPacket::GetPitch()
{
    return pitch;
}
uint32 MovementPacket::GetFallTime()
{
    return fallTime;
}

float MovementPacket::GetJumpZSpeed()
{
    return jumpZSpeed;
}
float MovementPacket::GetJumpSinAngle()
{
    return jumpSinAngle;
}
float MovementPacket::GetJumpCosAngle()
{
    return jumpCosAngle;
}
float MovementPacket::GetJumpXYSpeed()
{
    return jumpXYSpeed;
}
float MovementPacket::GetSplineElevation()
{
    return splineElevation;
}

MovementPacket& MovementPacket::SetGUID(uint64 value)
{
    GUID = value;
    return *this;
}
MovementPacket& MovementPacket::SetFlags(uint32 value)
{
    flags = value;
    return *this;
}
MovementPacket& MovementPacket::SetFlags2(uint16 value)
{
    flags2 = value;
    return *this;
}

MovementPacket& MovementPacket::SetX(float value)
{
    x = value;
    return *this;
}
MovementPacket& MovementPacket::SetY(float value)
{
    y = value;
    return *this;
}
MovementPacket& MovementPacket::SetZ(float value)
{
    z = value;
    return *this;
}
MovementPacket& MovementPacket::SetO(float value)
{
    o = value;
    return *this;
}
MovementPacket& MovementPacket::SetTime(uint32 value)
{
    time = value;
    return *this;
}

MovementPacket& MovementPacket::SetTransportGUID(uint64 value)
{
    TransportGUID = value;
    return *this;
}
MovementPacket& MovementPacket::SetTransportX(float value)
{
    transportX = value;
    return *this;
}
MovementPacket& MovementPacket::SetTransportY(float value)
{
    transportY = value;
    return *this;
}
MovementPacket& MovementPacket::SetTransportZ(float value)
{
    transportZ = value;
    return *this;
}
MovementPacket& MovementPacket::SetTransportO(float value)
{
    transportO = value;
    return *this;
}
MovementPacket& MovementPacket::SetTransportSeat(int8 value)
{
    transportSeat = value;
    return *this;
}
MovementPacket& MovementPacket::SetTransportTime(uint32 value)
{
    transportTime = value;
    return *this;
}
MovementPacket& MovementPacket::SetTransportTime2(uint32 value)
{
    transportTime2 = value;
    return *this;
}

MovementPacket& MovementPacket::SetPitch(float value)
{
    pitch = value;
    return *this;
}
MovementPacket& MovementPacket::SetFallTime(uint32 value)
{
    fallTime = value;
    return *this;
}

MovementPacket& MovementPacket::SetJumpZSpeed(float value)
{
    jumpZSpeed = value;
    return *this;
}
MovementPacket& MovementPacket::SetJumpSinAngle(float value)
{
    jumpSinAngle = value;
    return *this;
}
MovementPacket& MovementPacket::SetJumpCosAngle(float value)
{
    jumpCosAngle = value;
    return *this;
}
MovementPacket& MovementPacket::SetJumpXYSpeed(float value)
{
    jumpXYSpeed = value;
    return *this;
}
MovementPacket& MovementPacket::SetSplineElevation(float value)
{
    splineElevation = value;
    return *this;
}

void MovementPacket::Register(sol::state& state)
{
    auto LMovementPacket = state.new_usertype<MovementPacket>("MovementPacket");
    LMovementPacket.set_function("create", sol::overload(
        [](sol::object _, double value) {
            return MovementPacket::create(Opcodes(value));
        },
        [](double value) {
            return MovementPacket::create(Opcodes(value));
        }
    ));

    LMovementPacket.set_function("Write", &MovementPacket::Write);
    LMovementPacket.set_function("Send", &MovementPacket::Send);
    LMovementPacket.set_function("GetOpcode", &MovementPacket::GetOpcode);
    LMovementPacket.set_function("SetOpcode", &MovementPacket::SetOpcode);

    LMovementPacket.set_function("GetGUID", &MovementPacket::GetGUID);
    LMovementPacket.set_function("GetFlags", &MovementPacket::GetFlags);
    LMovementPacket.set_function("GetFlags2", &MovementPacket::GetFlags2);
    LMovementPacket.set_function("GetX", &MovementPacket::GetX);
    LMovementPacket.set_function("GetY", &MovementPacket::GetY);
    LMovementPacket.set_function("GetZ", &MovementPacket::GetZ);
    LMovementPacket.set_function("GetO", &MovementPacket::GetO);
    LMovementPacket.set_function("GetTime", &MovementPacket::GetTime);

    LMovementPacket.set_function("GetTransportGUID", &MovementPacket::GetTransportGUID);
    LMovementPacket.set_function("GetTransportX", &MovementPacket::GetTransportX);
    LMovementPacket.set_function("GetTransportY", &MovementPacket::GetTransportY);
    LMovementPacket.set_function("GetTransportZ", &MovementPacket::GetTransportZ);
    LMovementPacket.set_function("GetTransportO", &MovementPacket::GetTransportO);

    LMovementPacket.set_function("GetTransportSeat", &MovementPacket::GetTransportSeat);
    LMovementPacket.set_function("GetTransportTime", &MovementPacket::GetTransportTime);
    LMovementPacket.set_function("GetTransportTime2", &MovementPacket::GetTransportTime2);

    LMovementPacket.set_function("GetPitch", &MovementPacket::GetPitch);
    LMovementPacket.set_function("GetFallTime", &MovementPacket::GetFallTime);
    LMovementPacket.set_function("GetJumpZSpeed", &MovementPacket::GetJumpZSpeed);
    LMovementPacket.set_function("GetJumpSinAngle", &MovementPacket::GetJumpSinAngle);
    LMovementPacket.set_function("GetJumpCosAngle", &MovementPacket::GetJumpCosAngle);
    LMovementPacket.set_function("GetJumpXYSpeed", &MovementPacket::GetJumpXYSpeed);
    LMovementPacket.set_function("GetSplineElevation", &MovementPacket::GetSplineElevation);


    LMovementPacket.set_function("SetGUID", &MovementPacket::SetGUID);
    LMovementPacket.set_function("SetFlags", &MovementPacket::SetFlags);
    LMovementPacket.set_function("SetFlags2", &MovementPacket::SetFlags2);
    LMovementPacket.set_function("SetX", &MovementPacket::SetX);
    LMovementPacket.set_function("SetY", &MovementPacket::SetY);
    LMovementPacket.set_function("SetZ", &MovementPacket::SetZ);
    LMovementPacket.set_function("SetO", &MovementPacket::SetO);
    LMovementPacket.set_function("SetTime", &MovementPacket::SetTime);

    LMovementPacket.set_function("SetTransportGUID", &MovementPacket::SetTransportGUID);
    LMovementPacket.set_function("SetTransportX", &MovementPacket::SetTransportX);
    LMovementPacket.set_function("SetTransportY", &MovementPacket::SetTransportY);
    LMovementPacket.set_function("SetTransportZ", &MovementPacket::SetTransportZ);
    LMovementPacket.set_function("SetTransportO", &MovementPacket::SetTransportO);

    LMovementPacket.set_function("SetTransportSeat", &MovementPacket::SetTransportSeat);
    LMovementPacket.set_function("SetTransportTime", &MovementPacket::SetTransportTime);
    LMovementPacket.set_function("SetTransportTime2", &MovementPacket::SetTransportTime2);

    LMovementPacket.set_function("SetPitch", &MovementPacket::SetPitch);
    LMovementPacket.set_function("SetFallTime", &MovementPacket::SetFallTime);
    LMovementPacket.set_function("SetJumpZSpeed", &MovementPacket::SetJumpZSpeed);
    LMovementPacket.set_function("SetJumpSinAngle", &MovementPacket::SetJumpSinAngle);
    LMovementPacket.set_function("SetJumpCosAngle", &MovementPacket::SetJumpCosAngle);
    LMovementPacket.set_function("SetJumpXYSpeed", &MovementPacket::SetJumpXYSpeed);
    LMovementPacket.set_function("SetSplineElevation", &MovementPacket::SetSplineElevation);
}
