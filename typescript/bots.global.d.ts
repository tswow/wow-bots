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

declare class BotEvents {
    OnCreate(callback: (bot: Bot) => void): this;
    OnAuthChallenge(callback: (bot: Bot, packet: AuthPacket, cancel: BotMutable<boolean>) => void): this
    OnAuthProof(callback: (bot: Bot, challenge: ServerAuthChallenge, packet: AuthPacket, cancel: BotMutable<boolean>) => void): this
    OnRequestRealms(callback: (bot: Bot, packet: AuthPacket) => void): this
    OnSelectRealm(callback: (info: RealmInfo[], realm: BotMutable<RealmInfo>) => void): this
    OnCloseAuthConnection(callback: (bot: Bot, shouldClose: BotMutable<boolean>, cancel: BotMutable<boolean>) => void): this
    OnWorldAuthChallenge(callback: (bot: Bot, packetIn: WorldPacket, packetOut: WorldPacket, cancel: BotMutable<boolean>) => void): this
    OnWorldAuthResponse(callback: (bot: Bot, response: WorldAuthResponse, packetOut: WorldPacket, cancel: BotMutable<boolean>) => void): this
    OnWorldPacket(id: EventID, callback: (bot: Bot, packet: WorldPacket) => void): this
    OnWorldPacket(callback: (bot: Bot, packet: WorldPacket) => void): this
    Register(mod: string, name: string): this
}

declare const RootBot: BotEvents

declare function CreateAuthPacket(size?: number): AuthPacket
declare function CreateWorldPacket(opcode: Opcodes, size?: number): AuthPacket