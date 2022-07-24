
# Create declaration files
file(READ ${SDIR}/typescript/shared.global.d.ts SHARED_FILE)
file(READ ${SDIR}/typescript/bots.global.d.ts BOT_FILE)
file(READ ${SDIR}/typescript/commands.global.d.ts COMMANDS_FILE)
file(READ ${SDIR}/typescript/opcodes.global.d.ts OPCODES_FILE)

file(WRITE ${BDIR}/bots/global.d.ts "${OPCODES_FILE}")
file(WRITE ${BDIR}/commands/global.d.ts "${OPCODES_FILE}")

file(APPEND ${BDIR}/bots/global.d.ts "\n\n${SHARED_FILE}")
file(APPEND ${BDIR}/commands/global.d.ts "\n\n${SHARED_FILE}")

file(APPEND ${BDIR}/bots/global.d.ts "\n\n${BOT_FILE}")
file(APPEND ${BDIR}/commands/global.d.ts "\n\n${COMMANDS_FILE}")

# Create config file
file(READ ${SDIR}/bots.conf CONFIG_FILE)
file(WRITE ${BDIR}/bots.conf ${CONFIG_FILE})