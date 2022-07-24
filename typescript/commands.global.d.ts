declare class BotCommandArguments {
    GetNumber(index: number): number
    GetString(index: number): string
    GetBool(index: number): boolean
}
declare class BotCommandBuilder {
    AddStringParam(name: string, def?: string): BotCommandBuilder
    AddNumberParam(name: string, def?: number): BotCommandBuilder
    AddBoolParam(name: string, def?: number): BotCommandBuilder
    SetDescription(name: string): BotCommandBuilder
    SetCallback(callback: (args: BotCommandArguments) => void): void
}
declare function CreateCommand(name: string): BotCommandBuilder