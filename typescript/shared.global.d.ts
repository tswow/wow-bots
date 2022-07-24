declare function BOT_LOG_TRACE(category: string, fmt: string, ...values: any[]);
declare function BOG_LOG_DEBUG(category: string, fmt: string, ...values: any[]);
declare function BOT_LOG_INFO(category: string, fmt: string, ...values: any[]);
declare function BOT_LOG_WARN(category: string, fmt: string, ...values: any[]);
declare function BOT_LOG_ERROR(category: string, fmt: string, ...values: any[]);

declare function print(...values: any[])

declare function GetHeight(map: number, x: number, y: number): number
declare function GetVMapHeight(map: number, x: number, y: number): number
declare function IsInLineOfSight(map: number, x1: number, y1: number, z1: number, x2: number, y2: number, z2: number, ignoreFlags: number);

declare function ReadDir(dir: string): string[]
declare function ReadDirRecursive(dir: string): string[]
declare function IsFile(path: string): boolean
declare function IsDirectory(path: string): boolean
declare function WriteFile(path: string, value: string)
declare function ReadFile(path: string): string