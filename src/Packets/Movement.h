#pragma once

#include "PacketTypes.h"
#include "BotOpcodes.h"

class WorldPacket;
namespace sol { class state; }

enum MovementFlags : uint32
{
    MOVEMENTFLAG_NONE = 0x00000000,
    MOVEMENTFLAG_FORWARD = 0x00000001,
    MOVEMENTFLAG_BACKWARD = 0x00000002,
    MOVEMENTFLAG_STRAFE_LEFT = 0x00000004,
    MOVEMENTFLAG_STRAFE_RIGHT = 0x00000008,
    MOVEMENTFLAG_LEFT = 0x00000010,
    MOVEMENTFLAG_RIGHT = 0x00000020,
    MOVEMENTFLAG_PITCH_UP = 0x00000040,
    MOVEMENTFLAG_PITCH_DOWN = 0x00000080,
    MOVEMENTFLAG_WALKING = 0x00000100,
    MOVEMENTFLAG_ONTRANSPORT = 0x00000200,
    MOVEMENTFLAG_DISABLE_GRAVITY = 0x00000400,
    MOVEMENTFLAG_ROOT = 0x00000800,
    MOVEMENTFLAG_FALLING = 0x00001000,
    MOVEMENTFLAG_FALLING_FAR = 0x00002000,
    MOVEMENTFLAG_PENDING_STOP = 0x00004000,
    MOVEMENTFLAG_PENDING_STRAFE_STOP = 0x00008000,
    MOVEMENTFLAG_PENDING_FORWARD = 0x00010000,
    MOVEMENTFLAG_PENDING_BACKWARD = 0x00020000,
    MOVEMENTFLAG_PENDING_STRAFE_LEFT = 0x00040000,
    MOVEMENTFLAG_PENDING_STRAFE_RIGHT = 0x00080000,
    MOVEMENTFLAG_PENDING_ROOT = 0x00100000,
    MOVEMENTFLAG_SWIMMING = 0x00200000,
    MOVEMENTFLAG_ASCENDING = 0x00400000,
    MOVEMENTFLAG_DESCENDING = 0x00800000,
    MOVEMENTFLAG_CAN_FLY = 0x01000000,
    MOVEMENTFLAG_FLYING = 0x02000000,
    MOVEMENTFLAG_SPLINE_ELEVATION = 0x04000000,
    MOVEMENTFLAG_SPLINE_ENABLED = 0x08000000,
    MOVEMENTFLAG_WATERWALKING = 0x10000000,
    MOVEMENTFLAG_FALLING_SLOW = 0x20000000,
    MOVEMENTFLAG_HOVER = 0x40000000,

    MOVEMENTFLAG_MASK_MOVING =
    MOVEMENTFLAG_FORWARD | MOVEMENTFLAG_BACKWARD | MOVEMENTFLAG_STRAFE_LEFT | MOVEMENTFLAG_STRAFE_RIGHT |
    MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING_FAR | MOVEMENTFLAG_ASCENDING | MOVEMENTFLAG_DESCENDING |
    MOVEMENTFLAG_SPLINE_ELEVATION,

    MOVEMENTFLAG_MASK_TURNING =
    MOVEMENTFLAG_LEFT | MOVEMENTFLAG_RIGHT | MOVEMENTFLAG_PITCH_UP | MOVEMENTFLAG_PITCH_DOWN,

    MOVEMENTFLAG_MASK_MOVING_FLY =
    MOVEMENTFLAG_FLYING | MOVEMENTFLAG_ASCENDING | MOVEMENTFLAG_DESCENDING,

    /// @todo if needed: add more flags to this masks that are exclusive to players
    MOVEMENTFLAG_MASK_PLAYER_ONLY =
    MOVEMENTFLAG_FLYING,

    /// Movement flags that have change status opcodes associated for players
    MOVEMENTFLAG_MASK_HAS_PLAYER_STATUS_OPCODE = MOVEMENTFLAG_DISABLE_GRAVITY | MOVEMENTFLAG_ROOT |
    MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_WATERWALKING | MOVEMENTFLAG_FALLING_SLOW | MOVEMENTFLAG_HOVER
};

enum MovementFlags2 : uint32
{
    MOVEMENTFLAG2_NONE = 0x00000000,
    MOVEMENTFLAG2_NO_STRAFE = 0x00000001,
    MOVEMENTFLAG2_NO_JUMPING = 0x00000002,
    MOVEMENTFLAG2_UNK3 = 0x00000004,        // Overrides various clientside checks
    MOVEMENTFLAG2_FULL_SPEED_TURNING = 0x00000008,
    MOVEMENTFLAG2_FULL_SPEED_PITCHING = 0x00000010,
    MOVEMENTFLAG2_ALWAYS_ALLOW_PITCHING = 0x00000020,
    MOVEMENTFLAG2_UNK7 = 0x00000040,
    MOVEMENTFLAG2_UNK8 = 0x00000080,
    MOVEMENTFLAG2_UNK9 = 0x00000100,
    MOVEMENTFLAG2_UNK10 = 0x00000200,
    MOVEMENTFLAG2_INTERPOLATED_MOVEMENT = 0x00000400,
    MOVEMENTFLAG2_INTERPOLATED_TURNING = 0x00000800,
    MOVEMENTFLAG2_INTERPOLATED_PITCHING = 0x00001000,
    MOVEMENTFLAG2_UNK14 = 0x00002000,
    MOVEMENTFLAG2_CAN_TRANSITION_BETWEEN_SWIM_AND_FLY = 0x00004000,
    MOVEMENTFLAG2_UNK16 = 0x00008000
};

class Bot;
class MovementPacket
{
public:
    static MovementPacket Read(WorldPacket& packet);
    static void Register(sol::state& state);
    static MovementPacket create(Opcodes opcodes);
    WorldPacket Write();

    void Send(Bot& bot);

    Opcodes GetOpcode();
    MovementPacket& SetOpcode(Opcodes opcodes);

    uint64 GetGUID();
    uint32 GetFlags();
    uint16 GetFlags2();

    float GetX();
    float GetY();
    float GetZ();
    float GetO();
    uint32 GetTime();

    uint64 GetTransportGUID();
    float GetTransportX();
    float GetTransportY();
    float GetTransportZ();
    float GetTransportO();

    int8 GetTransportSeat();
    uint32 GetTransportTime();
    uint32 GetTransportTime2();

    float GetPitch();
    uint32 GetFallTime();

    float GetJumpZSpeed();
    float GetJumpSinAngle(); 
    float GetJumpCosAngle();
    float GetJumpXYSpeed();
    float GetSplineElevation();

    MovementPacket& SetGUID(uint64 value);
    MovementPacket& SetFlags(uint32 value);
    MovementPacket& SetFlags2(uint16 value);

    MovementPacket& SetX(float value);
    MovementPacket& SetY(float value);
    MovementPacket& SetZ(float value);
    MovementPacket& SetO(float value);
    MovementPacket& SetTime(uint32 value);

    MovementPacket& SetTransportGUID(uint64 value);
    MovementPacket& SetTransportX(float value);
    MovementPacket& SetTransportY(float value);
    MovementPacket& SetTransportZ(float value);
    MovementPacket& SetTransportO(float value);

    MovementPacket& SetTransportSeat(int8 value);
    MovementPacket& SetTransportTime(uint32 value);
    MovementPacket& SetTransportTime2(uint32 value);

    MovementPacket& SetPitch(float value);
    MovementPacket& SetFallTime(uint32 value);

    MovementPacket& SetJumpZSpeed(float value);
    MovementPacket& SetJumpSinAngle(float value);
    MovementPacket& SetJumpCosAngle(float value);
    MovementPacket& SetJumpXYSpeed(float value);
    MovementPacket& SetSplineElevation(float value);
private:
    Opcodes opcode;
    uint64 GUID;
    uint32 flags;
    uint16 flags2;
    float x;
    float y;
    float z;
    float o;
    uint32 time;

    uint64 TransportGUID;
    float transportX;
    float transportY;
    float transportZ;
    float transportO;
    int8 transportSeat;
    uint32 transportTime;
    uint32 transportTime2;
    float pitch;
    uint32 fallTime;

    float jumpZSpeed;
    float jumpSinAngle;
    float jumpCosAngle;
    float jumpXYSpeed;

    float splineElevation;
};