#pragma once

#include "packet-builder.h"

PacketBuilder* CharEnumItem = PacketBuilder::CreateChunk(PacketType::READ, "CharEnumItem")
    ->SingleField("uint32", "DisplayID")
    ->SingleField("uint8", "InventoryType")
    ->SingleField("uint32", "Padding")
;

PacketBuilder* CharEnumBag = PacketBuilder::CreateChunk(PacketType::READ, "CharEnumBag")
    ->SingleField("uint32","V1")
    ->SingleField("uint32","V2")
    ->SingleField("uint32","V3")
    ;

PacketBuilder* CharEnumEntry = PacketBuilder::CreateChunk(PacketType::READ, "CharEnumEntry")
    ->SingleField("uint64","GUID")
    ->SingleField(CString(), "Name")
    ->SingleField("uint8","Race")
    ->SingleField("uint8","Class")
    ->SingleField("uint8","Gender")
    ->ArrayField("uint8","Padding",5)
    ->SingleField("uint8","Level")
    ->SingleField("uint32","ZoneID")
    ->SingleField("uint32","MapID")
    ->SingleField("float","X")
    ->SingleField("float","Y")
    ->SingleField("float","Z")
    ->SingleField("uint32","Guild")
    ->SingleField("uint32","Flags")
    ->SingleField("uint32","Customize")
    ->SingleField("uint8","FirstLogin")
    ->SingleField("uint32","PetInfoID")
    ->SingleField("uint32","PetLevel")
    ->SingleField("uint32","PetFamilyID")
    ->ArrayField(CharEnumItem,"Item",19)
    ->ArrayField(CharEnumBag,"Bag",4)
    ;

PacketBuilder* SMSGCharEnum = PacketBuilder
    ::CreatePacket(Opcodes::SMSG_CHAR_ENUM, PacketType::READ, "SMSGCharEnum")
    ->VectorField(CharEnumEntry, "Entry", "uint8")
    ;

PacketBuilder* CMSGCharEnum = PacketBuilder
    ::CreatePacket(Opcodes::CMSG_CHAR_ENUM, PacketType::WRITE, "CMSGCharEnum")
    ;

PacketBuilder* CMSGPlayerLogin = PacketBuilder
    ::CreatePacket(Opcodes::CMSG_PLAYER_LOGIN,PacketType::WRITE, "CMSGPlayerLogin")
    ->SingleField("uint64", "GUID")
    ;
