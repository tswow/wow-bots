declare class BotCommandArguments {
    GetNumber(name: string): number
    GetString(name: string): string
    GetBool(name: string): boolean
}
declare class BotCommandBuilder {
    AddStringParam(name: string, def?: string): BotCommandBuilder
    AddNumberParam(name: string, def?: number): BotCommandBuilder
    AddBoolParam(name: string, def?: number): BotCommandBuilder
    SetDescription(name: string): BotCommandBuilder
    SetCallback(callback: (args: BotCommandArguments) => void): void
}
declare function CreateCommand(name: string): BotCommandBuilder

declare function StartBot(username: string, password: string, )

declare class BotAccount {
    GetUsername(): string;
    GetPassword(): string
}
declare function GetBotAccount(id: number): BotAccount;
declare function GetBotAccounts(ids: number[]): BotAccount[];

