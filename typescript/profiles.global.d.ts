declare type EventID = number | number[];
declare class PacketBase {
    WriteBytes(arr: number[]): this;
    WriteString(str: string): this;
    WriteUInt8(value: number): this;
    WriteInt8(value: number): this;
    WriteUInt16(value: number): this;
    WriteInt16(value: number): this;
    WriteUInt32(value: number): this;
    WriteInt32(value: number): this;
    WriteUInt64(value: number): this;
    WriteInt64(value: number): this;
    ReadBytes(amount: number): number[]
    ReadString(size: number): string
    ReadCString(): string
    ReadUInt8(): number
    ReadInt8(): number
    ReadUInt16(): number
    ReadInt16(): number
    ReadUInt32(): number
    ReadInt32(): number
    ReadUInt64(): number
    ReadInt64(): number
    ReadFloat(): number
    ReadDouble(): number
    Reserve(amount: number): this
    Seek(pos: number): this
    Reset(): this
    Send(bot: Bot): void
}

declare class WorldPacket extends PacketBase {
    GetOpcode(): Opcodes
    SetOpcode(opcode: Opcodes): this
    Send(bot: Bot): void
    GetPayloadSize(): number
    ReadPackedGUID(): number;
}
declare class AuthPacket extends PacketBase {}

declare class Bot {
    DisconnectNow(): void
    QueueDisconnect(): void
    GetUsername(): string
    GetPassword(): string
    GetEvents(): BotEvents
    GetData<T>(key: string, def?: T): T
    SetData<T>(key: string, value: T): Bot
}


declare class BotMutable<T> {
    set(value: T): void
    get(): T
}

declare class ServerAuthChallenge {}
declare class RealmInfo {
    type: number
    locked: number
    flags: number
    name: string
    address: string
    port: number
    population: number
    load: number
    timezone: number
    id: number
    major_version: number
    minor_version: number
    bugfix_version: number
    build: number
}
declare class WorldAuthResponse {}

declare class BotProfile {
    SetBehaviorRoot(node: RootNode<Bot,void,void>)
    OnLoad(callback: (bot: Bot) => void): BotProfile;
    OnAuthChallenge(callback: (bot: Bot, packet: AuthPacket, cancel: BotMutable<boolean>) => void): BotProfile
    OnAuthProof(callback: (bot: Bot, challenge: ServerAuthChallenge, packet: AuthPacket, cancel: BotMutable<boolean>) => void): BotProfile
    OnRequestRealms(callback: (bot: Bot, packet: AuthPacket) => void): BotProfile
    OnSelectRealm(callback: (info: RealmInfo[], realm: BotMutable<RealmInfo>) => void): BotProfile
    OnCloseAuthConnection(callback: (bot: Bot, shouldClose: BotMutable<boolean>, cancel: BotMutable<boolean>) => void): BotProfile
    OnWorldAuthChallenge(callback: (bot: Bot, packetIn: WorldPacket, packetOut: WorldPacket, cancel: BotMutable<boolean>) => void): BotProfile
    OnWorldAuthResponse(callback: (bot: Bot, response: WorldAuthResponse, packetOut: WorldPacket, cancel: BotMutable<boolean>) => void): BotProfile
    OnWorldPacket(id: EventID, callback: (bot: Bot, packet: WorldPacket) => void): BotProfile
    OnWorldPacket(callback: (bot: Bot, packet: WorldPacket) => void): BotProfile
    OnMovementPacket(callback: (bot: Bot, packet: MovementPacket) => void): BotProfile
    OnUpdateData(callback: (bot: Bot, packet: UpdateDataPacket) => void): BotProfile
    Register(mod: string, name: string): BotProfile
}

declare class MovementPacket {
    Send(bot: Bot): void;
    static create(opcodes: Opcodes): MovementPacket

    GetOpcode(): Opcodes;
    SetOpcode(opcodes: Opcodes): MovementPacket;

    GetGUID(): uint64;
    GetFlags(): uint32;
    GetFlags2(): uint16;

    GetX(): float;
    GetY(): float;
    GetZ(): float;
    GetO(): float;
    GetTime(): uint32;

    GetTransportGUID(): uint64;
    GetTransportX(): float;
    GetTransportY(): float;
    GetTransportZ(): float;
    GetTransportO(): float;

    GetTransportSeat(): int8;
    GetTransportTime(): uint32;
    GetTransportTime2(): uint32;

    GetPitch(): float;
    GetFallTime(): uint32;

    GetJumpZSpeed(): float;
    GetJumpSinAngle(): float;
    GetJumpCosAngle(): float;
    GetJumpXYSpeed(): float;
    GetSplineElevation(): float;

    SetGUID(value: uint64): MovementPacket;
    SetFlags(value: uint32): MovementPacket;
    SetFlags2(value: uint16): MovementPacket;

    SetX(value: float): MovementPacket;
    SetY(value: float): MovementPacket;
    SetZ(value: float): MovementPacket;
    SetO(value: float): MovementPacket;
    SetTime(value: uint32): MovementPacket;

    SetTransportGUID(value: uint64): MovementPacket;
    SetTransportX(value: float): MovementPacket;
    SetTransportY(value: float): MovementPacket;
    SetTransportZ(value: float): MovementPacket;
    SetTransportO(value: float): MovementPacket;

    SetTransportSeat(value: int8): MovementPacket;
    SetTransportTime(value: uint32): MovementPacket;
    SetTransportTime2(value: uint32): MovementPacket;

    SetPitch(value: float): MovementPacket;
    SetFallTime(value: uint32): MovementPacket;

    SetJumpZSpeed(value: float): MovementPacket;
    SetJumpSinAngle(value: float): MovementPacket;
    SetJumpCosAngle(value: float): MovementPacket;
    SetJumpXYSpeed(value: float): MovementPacket;
    SetSplineElevation(value: float): MovementPacket;
}

declare enum TypeID
{
    TYPEID_OBJECT        = 0,
    TYPEID_ITEM          = 1,
    TYPEID_CONTAINER     = 2,
    TYPEID_UNIT          = 3,
    TYPEID_PLAYER        = 4,
    TYPEID_GAMEOBJECT    = 5,
    TYPEID_DYNAMICOBJECT = 6,
    TYPEID_CORPSE        = 7
}

declare enum UnitMoveType
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
}

declare enum SplineEvaluationMode
{
    ModeLinear,
    ModeCatmullrom,
    ModeBezier3_Unused,
    UninitializedMode,
    ModesEnd
}

declare enum ObjectUpdateFlags
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
}

declare enum ObjectUpdateType
{
    UPDATETYPE_VALUES               = 0,
    UPDATETYPE_MOVEMENT             = 1,
    UPDATETYPE_CREATE_OBJECT        = 2,
    UPDATETYPE_CREATE_OBJECT2       = 3,
    UPDATETYPE_OUT_OF_RANGE_OBJECTS = 4,
    UPDATETYPE_NEAR_OBJECTS         = 5
}

declare enum SplineFlags
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
}

declare class Vector3 {
    GetX(): float
    GetY(): float
    GetZ(): float
}

declare class UpdateData {
    GetUpdateType(): ObjectUpdateType;
    GetGUID(): uint64;
    GetTypeID(): TypeID;
    GetUpdateFlags(): ObjectUpdateFlags;
    GetMovement(): MovementPacket;
    GetWalkSpeed(): float;
    GetRunSpeed(): float;
    GetRunBackSpeed(): float;
    GetSwimSpeed(): float;
    GetSwimBackSpeed(): float;
    GetFlightSpeed(): float;
    GetFlightBackSpeed(): float;
    GetTurnRate(): float;
    GetPitchRate(): float;
    GetSplineFlags(): SplineFlags;
    GetSplineFacingAngle(): float;
    GetSplineFacingTargetGUID(): uint64;
    GetSplineFacingPointX(): float;
    GetSplineFacingPointY(): float;
    GetSplineFacingPointZ(): float;
    GetSplineTimePassed(): int32;
    GetSplineDuration(): int32;
    GetSplineID(): uint32;
    GetSplineVerticalAcceleration(): float;
    GetSplineEffectStartTime(): int;
    GetSplinePoint(index: number): Vector3;
    GetSplinePointCount(): uint32;
    GetSplineEvaluationMode(): SplineEvaluationMode;
    GetSplineEndpointX(): float;
    GetSplineEndpointY(): float;
    GetSplineEndpointZ(): float;
    GetTransportGUID(): uint64;
    GetX(): float;
    GetY(): float;
    GetZ(): float;
    GetTransportOffsetX(): float;
    GetTransportOffsetY(): float;
    GetTransportOffsetZ(): float;
    GetO(): float;
    GetCorpseOrientation(): float;
    GetLowGUID(): uint32;
    GetTargetGUID(): uint64;
    GetTransportTimer(): uint32;
    GetVehicleID(): uint32;
    GetVehicleOrientation(): float;
    GetGORotation(): uint64;
    GetUpdateFields(): {[key: number]: number};
    GetUpdateField(field: int32): uint32;
    HasUpdateField(field: int32): boolean;
    GetOutOfRangeGUID(index: uint32): uint64;
    OutOfRangeGUIDCount(): uint32;
}

declare class UpdateDataPacket {
    GetEntry(index: number): UpdateData;
    EntryCount(): number;
}

declare const RootBot: BotProfile
declare function CreateBotProfile(): BotProfile

declare function CreateAuthPacket(size?: number): AuthPacket
declare function CreateWorldPacket(opcode: Opcodes, size?: number): WorldPacket

// ============================================================================
// Behavior Tree
// ============================================================================
declare function BotCreateLeaf(callback: LeafCallback<Bot,monostate>): Leaf<Bot,monostate,monostate>
declare function BotCreateMultiplexer(callback?: LeafCallback<Bot,monostate>): Multiplexer<Bot,monostate,monostate>
declare function BotCreateSequence(): Branch<Bot,monostate,monostate>
declare function BotCreateSelector(): Branch<Bot,monostate,monostate>