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
}
declare class AuthPacket extends PacketBase {}

declare class Bot {
    DisconnectNow(): void
    QueueDisconnect(): void
    GetUsername(): string
    GetPassword(): string
    GetEvents(): BotEvents
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
    Register(mod: string, name: string): BotProfile
}
declare const RootBot: BotProfile
declare function CreateBotProfile(): BotProfile

declare function CreateAuthPacket(size?: number): AuthPacket
declare function CreateWorldPacket(opcode: Opcodes, size?: number): AuthPacket

// ============================================================================
// Behavior Tree
// ============================================================================
declare function BotCreateLeaf(callback: LeafCallback<Bot,monostate>): Leaf<Bot,monostate,monostate>
declare function BotCreateMultiplexer(callback?: LeafCallback<Bot,monostate>): Multiplexer<Bot,monostate,monostate>
declare function BotCreateSequence(): Branch<Bot,monostate,monostate>
declare function BotCreateSelector(): Branch<Bot,monostate,monostate>