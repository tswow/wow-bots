
# Create declaration files
file(READ ${SDIR}/typescript/shared.global.d.ts SHARED_FILE)
file(READ ${SDIR}/typescript/profiles.global.d.ts PROFILES_FILE)
file(READ ${SDIR}/typescript/commands.global.d.ts COMMANDS_FILE)
file(READ ${SDIR}/typescript/opcodes.global.d.ts OPCODES_FILE)
file(READ ${SDIR}/typescript/packets.global.d.ts PACKETS_FILE)

file(READ ${BTSDIR}/behaviortree.global.d.ts BEHAVIORTREE_FILE)

file(WRITE ${BDIR}/profiles/global.d.ts "${OPCODES_FILE}")
file(WRITE ${BDIR}/commands/global.d.ts "${OPCODES_FILE}")

file(APPEND ${BDIR}/profiles/global.d.ts "\n\n${SHARED_FILE}")
file(APPEND ${BDIR}/commands/global.d.ts "\n\n${SHARED_FILE}")

file(APPEND ${BDIR}/profiles/global.d.ts "\n\n${BEHAVIORTREE_FILE}")

file(APPEND ${BDIR}/profiles/global.d.ts "\n\n${PROFILES_FILE}")
file(APPEND ${BDIR}/commands/global.d.ts "\n\n${COMMANDS_FILE}")

file(APPEND ${BDIR}/profiles/global.d.ts "\n\n${PACKETS_FILE}")

# Create config file
file(READ ${SDIR}/bots.conf CONFIG_FILE)
file(WRITE ${BDIR}/bots.conf ${CONFIG_FILE})