#pragma

#include "PacketTypes.h"
#include "Movement.h"

#include <vector>
#include <map>

class WorldPacket;
namespace sol { class state; }

enum TypeID
{
    TYPEID_OBJECT        = 0,
    TYPEID_ITEM          = 1,
    TYPEID_CONTAINER     = 2,
    TYPEID_UNIT          = 3,
    TYPEID_PLAYER        = 4,
    TYPEID_GAMEOBJECT    = 5,
    TYPEID_DYNAMICOBJECT = 6,
    TYPEID_CORPSE        = 7
};

enum UnitMoveType
{
    MOVE_WALK        = 0,
    MOVE_RUN         = 1,
    MOVE_RUN_BACK    = 2,
    MOVE_SWIM        = 3,
    MOVE_SWIM_BACK   = 4,
    MOVE_TURN_RATE   = 5,
    MOVE_FLIGHT      = 6,
    MOVE_FLIGHT_BACK = 7,
    MOVE_PITCH_RATE  = 8
};

enum ObjectUpdateFlags
{
    UPDATEFLAG_NONE = 0x0000,
    UPDATEFLAG_SELF = 0x0001,
    UPDATEFLAG_TRANSPORT = 0x0002,
    UPDATEFLAG_HAS_TARGET = 0x0004,
    UPDATEFLAG_UNKNOWN = 0x0008,
    UPDATEFLAG_LOWGUID = 0x0010,
    UPDATEFLAG_LIVING = 0x0020,
    UPDATEFLAG_STATIONARY_POSITION = 0x0040,
    UPDATEFLAG_VEHICLE = 0x0080,
    UPDATEFLAG_POSITION = 0x0100,
    UPDATEFLAG_ROTATION = 0x0200
};

enum ObjectUpdateType
{
    UPDATETYPE_VALUES               = 0,
    UPDATETYPE_MOVEMENT             = 1,
    UPDATETYPE_CREATE_OBJECT        = 2,
    UPDATETYPE_CREATE_OBJECT2       = 3,
    UPDATETYPE_OUT_OF_RANGE_OBJECTS = 4,
    UPDATETYPE_NEAR_OBJECTS         = 5
};

enum SplineFlags
{
    SPLINEFLAGS_NONE = 0x00000000,
    SPLINEFLAGS_DONE = 0x00000100,
    SPLINEFLAGS_FALLING = 0x00000200,
    SPLINEFLAGS_NO_SPLINE = 0x00000400,
    SPLINEFLAGS_PARABOLIC = 0x00000800,
    SPLINEFLAGS_WALKMODE = 0x00001000,
    SPLINEFLAGS_FLYING = 0x00002000,
    SPLINEFLAGS_ORIENTATION_FIXED = 0x00004000,
    SPLINEFLAGS_FINAL_POINT = 0x00008000,
    SPLINEFLAGS_FINAL_TARGET = 0x00010000,
    SPLINEFLAGS_FINAL_ANGLE = 0x00020000,
    SPLINEFLAGS_CATMULLROM = 0x00040000,
    SPLINEFLAGS_CYCLIC = 0x00080000,
    SPLINEFLAGS_ENTER_CYCLE = 0x00100000,
    SPLINEFLAGS_ANIMATION = 0x00200000,
    SPLINEFLAGS_FROZEN = 0x00400000,
    SPLINEFLAGS_TRANSPORT_ENTER = 0x00800000,
    SPLINEFLAGS_TRANSPORT_EXIT = 0x01000000,
    SPLINEFLAGS_UNKNOWN_7 = 0x02000000,
    SPLINEFLAGS_UNKNOWN_8 = 0x04000000,
    SPLINEFLAGS_ORIENTATION_INVERSED = 0x08000000,
    SPLINEFLAGS_UNKNOWN_10 = 0x10000000,
    SPLINEFLAGS_UNKNOWN_11 = 0x20000000,
    SPLINEFLAGS_UNKNOWN_12 = 0x40000000,
    SPLINEFLAGS_UNKNOWN_13 = 0x80000000,

    SPLINEFLAGS_MASK_FINAL_FACING = SPLINEFLAGS_FINAL_POINT | SPLINEFLAGS_FINAL_TARGET | SPLINEFLAGS_FINAL_ANGLE,
    SPLINEFLAGS_MASK_ANIMATIONS = 0xFF,
    SPLINEFLAGS_MASK_NO_MONSTER_MOVE = SPLINEFLAGS_MASK_FINAL_FACING | SPLINEFLAGS_MASK_ANIMATIONS| SPLINEFLAGS_DONE,
    SPLINEFLAGS_MASK_CATMULLROM = SPLINEFLAGS_FLYING | SPLINEFLAGS_CATMULLROM,
    SPLINEFLAGS_MASK_UNUSED = SPLINEFLAGS_NO_SPLINE | SPLINEFLAGS_ENTER_CYCLE | SPLINEFLAGS_FROZEN | SPLINEFLAGS_UNKNOWN_7 | SPLINEFLAGS_UNKNOWN_8 | SPLINEFLAGS_UNKNOWN_10 | SPLINEFLAGS_UNKNOWN_11 | SPLINEFLAGS_UNKNOWN_12 | SPLINEFLAGS_UNKNOWN_13
};

class Vector3
{
public:
    Vector3(float x, float y, float z);
    Vector3() = default;
    float GetX();
    float GetY();
    float GetZ();
private:
    float x;
    float y;
    float z;
};

enum class SplineEvaluationMode
{
    ModeLinear,
    ModeCatmullrom,
    ModeBezier3_Unused,
    UninitializedMode,
    ModesEnd
};

class UpdateData
{
public:
    static UpdateData Read(WorldPacket& packet);
    static void Register(sol::state& state);
    ObjectUpdateType GetUpdateType();
    uint64 GetGUID();
    TypeID GetTypeID();
    ObjectUpdateFlags GetUpdateFlags();
    MovementPacket& GetMovement();
    float GetWalkSpeed();
    float GetRunSpeed();
    float GetRunBackSpeed();
    float GetSwimSpeed();
    float GetSwimBackSpeed();
    float GetFlightSpeed();
    float GetFlightBackSpeed();
    float GetTurnRate();
    float GetPitchRate();
    SplineFlags GetSplineFlags();
    float GetSplineFacingAngle();
    uint64 GetSplineFacingTargetGUID();
    float GetSplineFacingPointX();
    float GetSplineFacingPointY();
    float GetSplineFacingPointZ();
    int32 GetSplineTimePassed();
    int32 GetSplineDuration();
    uint32 GetSplineID();
    float GetSplineVerticalAcceleration();
    int GetSplineEffectStartTime();
    Vector3 GetSplinePoint(int index);
    uint32 GetSplinePointCount();
    SplineEvaluationMode GetSplineEvaluationMode();
    float GetSplineEndpointX();
    float GetSplineEndpointY();
    float GetSplineEndpointZ();
    uint64 GetTransportGUID();
    float GetX();
    float GetY();
    float GetZ();
    float GetTransportOffsetX();
    float GetTransportOffsetY();
    float GetTransportOffsetZ();
    float GetO();
    float GetCorpseOrientation();
    uint32 GetLowGUID();
    uint64 GetTargetGUID();
    uint32 GetTransportTimer();
    uint32 GetVehicleID();
    float GetVehicleOrientation();
    uint64 GetGORotation();
    std::map<int32, uint32>& GetUpdateFields();
    uint32 GetUpdateField(int32 field);
    bool HasUpdateField(int32 field);
    uint64 GetOutOfRangeGUID(uint32 index);
    uint32 OutOfRangeGUIDCount();
private:
    void ReadValues(WorldPacket& packet);
    void ReadMovement(WorldPacket& packet);
    ObjectUpdateType updateType;
    uint64 GUID;
    TypeID typeID;
    ObjectUpdateFlags flags;
    MovementPacket movement;
    float moveWalkSpeed;
    float moveRunSpeed;
    float moveRunBackSpeed;
    float moveSwimSpeed;
    float moveSwimBackSpeed;
    float moveFlightSpeed;
    float moveFlightBackSpeed;
    float moveTurnRate;
    float movePitchRate;
    SplineFlags splineFlags;
    float splineFacingAngle;
    uint64 splineFacingTargetGUID;
    float splineFacingPointX;
    float splineFacingPointY;
    float splineFacingPointZ;
    int32 splineTimePassed;
    int32 splineDuration;
    uint32 splineId;
    float splineVerticalAcceleration;
    int splineEffectStartTime;
    std::vector<Vector3> splinePoints;
    SplineEvaluationMode splineEvaluationMode;
    float splineEndpointX;
    float splineEndpointY;
    float splineEndpointZ;
    uint64 transportGuid;
    float x;
    float y;
    float z;
    float transportOffsetX;
    float transportOffsetY;
    float transportOffsetZ;
    float o;
    float corpseOrientation;
    uint32 lowGuid;
    uint64 targetGuid;
    uint32 transportTimer;
    uint32 vehicleID;
    float vehicleOrientation;
    uint64 goRotation;
    std::map<int32, uint32> updateFields;
    std::vector<uint64> outOfRangeGuids;
};

class UpdateDataPacket
{
public:
    static UpdateDataPacket Read(WorldPacket& packet);
    static UpdateDataPacket ReadCompressed(WorldPacket& packet);
    UpdateData& GetEntry(uint32 entry);
    uint32 EntryCount();
private:
    std::vector<UpdateData> Entries;
};