#include "Update.h"
#include "BotPacket.h"

#include <sol/sol.hpp>

#include <zlib.h>

Vector3::Vector3(float _x, float _y, float _z)
    : x(_x), y(_y), z(_z)
{}

float Vector3::GetX()
{
    return x;
}

float Vector3::GetY()
{
    return y;
}

float Vector3::GetZ()
{
    return z;
}

void UpdateData::ReadValues(WorldPacket& packet)
{
    uint8 blockCount = packet.ReadUInt8();
    std::vector<int> updateMask(blockCount,0);
    for (uint8 i = 0; i < blockCount; ++i)
    {
        updateMask[i] = packet.ReadInt32();
    }
    
    for (size_t chunkI = 0; chunkI < updateMask.size(); ++chunkI)
    {
        for (size_t byteI = 0; byteI < 32; ++byteI)
        {
            if (updateMask[chunkI] & (1 << byteI))
            {
                updateFields[chunkI << 32 & byteI] = packet.ReadUInt32();
            }
        }
    }
}

void UpdateData::ReadMovement(WorldPacket& packet)
{
    flags = (ObjectUpdateFlags)packet.ReadUInt16();
    if (flags & UPDATEFLAG_LIVING)
    {
        movement.Read(packet);
        moveWalkSpeed = packet.ReadFloat();
        moveRunSpeed = packet.ReadFloat();
        moveRunBackSpeed= packet.ReadFloat();
        moveSwimSpeed = packet.ReadFloat();
        moveSwimBackSpeed = packet.ReadFloat();
        moveFlightSpeed = packet.ReadFloat();
        moveFlightBackSpeed = packet.ReadFloat();
        moveTurnRate = packet.ReadFloat();
        movePitchRate = packet.ReadFloat();
        if (movement.GetFlags() & MovementFlags::MOVEMENTFLAG_SPLINE_ENABLED)
        {
            splineFlags = SplineFlags(packet.ReadUInt32());
            if (splineFlags & SPLINEFLAGS_FINAL_ANGLE)
            {
                splineFacingAngle = packet.ReadFloat();
            }
            else if (splineFlags & SPLINEFLAGS_FINAL_TARGET)
            {
                splineFacingTargetGUID = packet.ReadUInt64();
            }
            else if (splineFlags & SPLINEFLAGS_FINAL_POINT)
            {
                splineFacingPointX = packet.ReadFloat();
                splineFacingPointY = packet.ReadFloat();
                splineFacingPointZ = packet.ReadFloat();
            }

            splineTimePassed = packet.ReadInt32();
            splineDuration = packet.ReadInt32();
            splineId = packet.ReadUInt32();
            packet.ReadFloat();
            packet.ReadFloat();
            splineVerticalAcceleration = packet.ReadFloat();
            splineEffectStartTime = packet.ReadInt32();
            uint32 splineCount = packet.ReadUInt32();
            splinePoints.resize(splineCount);
            for (uint32 i = 0; i < splineCount; ++i)
            {
                float x = packet.ReadFloat();
                float y = packet.ReadFloat();
                float z = packet.ReadFloat();
                splinePoints[i] = { x,y,z };
            }
            splineEvaluationMode = SplineEvaluationMode(packet.ReadUInt8());
            splineEndpointX = packet.ReadFloat();
            splineEndpointY = packet.ReadFloat();
            splineEndpointZ = packet.ReadFloat();
        }
    }
    else if (flags & UPDATEFLAG_POSITION)
    {
        transportGuid = packet.ReadPackedGUID();
        x = packet.ReadFloat();
        y = packet.ReadFloat();
        z = packet.ReadFloat();
        transportOffsetX = packet.ReadFloat();
        transportOffsetY = packet.ReadFloat();
        transportOffsetZ = packet.ReadFloat();
        o = packet.ReadFloat();
        corpseOrientation = packet.ReadFloat();
    }
    else if (flags & UPDATEFLAG_STATIONARY_POSITION)
    {
        x = packet.ReadFloat();
        y = packet.ReadFloat();
        z = packet.ReadFloat();
        o = packet.ReadFloat();
    }

    if (flags & UPDATEFLAG_UNKNOWN)
    {
        packet.ReadUInt32();
    }

    if (flags & UPDATEFLAG_LOWGUID)
    {
        lowGuid = packet.ReadUInt32();
    }

    if (flags & UPDATEFLAG_HAS_TARGET)
    {
        targetGuid = packet.ReadPackedGUID();
    }

    if (flags & UPDATEFLAG_TRANSPORT)
    {
        transportTimer = packet.ReadUInt32();
    }

    if (flags & UPDATEFLAG_VEHICLE)
    {
        vehicleID = packet.ReadUInt32();
        vehicleOrientation = packet.ReadFloat();
    }

    if (flags & UPDATEFLAG_ROTATION)
    {
        goRotation = packet.ReadInt64();
    }
}

UpdateData UpdateData::Read(WorldPacket& packet)
{
    UpdateData data;
    ObjectUpdateType type = ObjectUpdateType(packet.ReadUInt8());
    switch (type)
    {
    case ObjectUpdateType::UPDATETYPE_VALUES:
    {
        data.GUID = packet.ReadPackedGUID();
        data.ReadValues(packet);
        break;
    }
    case ObjectUpdateType::UPDATETYPE_MOVEMENT:
    {
        data.GUID = packet.ReadPackedGUID();
        data.ReadMovement(packet);
        break;
    }
    case ObjectUpdateType::UPDATETYPE_CREATE_OBJECT:
    case ObjectUpdateType::UPDATETYPE_CREATE_OBJECT2:
    {
        data.GUID = packet.ReadPackedGUID();
        data.typeID = TypeID(packet.ReadUInt8());
        data.ReadMovement(packet);
        data.ReadValues(packet);
        break;
    }
    case ObjectUpdateType::UPDATETYPE_OUT_OF_RANGE_OBJECTS:
    {
        uint32 guidCount = packet.ReadUInt32();
        for (uint32_t i = 0; i < guidCount; ++i)
        {
            data.outOfRangeGuids.push_back(packet.ReadUInt32());
        }
        break;
    }
    case ObjectUpdateType::UPDATETYPE_NEAR_OBJECTS:
    {
        break;
    }
    }
    return data;
}

ObjectUpdateType UpdateData::GetUpdateType()
{
    return updateType;
}

uint64 UpdateData::GetGUID()
{
    return GUID;
}

TypeID UpdateData::GetTypeID()
{
    return typeID;
}

ObjectUpdateFlags UpdateData::GetUpdateFlags()
{
    return flags;
}

MovementPacket& UpdateData::GetMovement()
{
    return movement;
}

float UpdateData::GetWalkSpeed()
{
    return moveWalkSpeed;
}

float UpdateData::GetRunSpeed()
{
    return moveRunSpeed;
}

float UpdateData::GetRunBackSpeed()
{
    return moveRunBackSpeed;
}

float UpdateData::GetSwimSpeed()
{
    return moveSwimSpeed;
}

float UpdateData::GetSwimBackSpeed()
{
    return moveSwimBackSpeed;
}

float UpdateData::GetFlightSpeed()
{
    return moveFlightSpeed;
}

float UpdateData::GetFlightBackSpeed()
{
    return moveFlightBackSpeed;
}

float UpdateData::GetTurnRate()
{
    return moveTurnRate;
}

float UpdateData::GetPitchRate()
{
    return movePitchRate;
}

SplineFlags UpdateData::GetSplineFlags()
{
    return splineFlags;
}

float UpdateData::GetSplineFacingAngle()
{
    return splineFacingAngle;
}

uint64 UpdateData::GetSplineFacingTargetGUID()
{
    return splineFacingTargetGUID;
}

float UpdateData::GetSplineFacingPointX()
{
    return splineFacingPointX;
}

float UpdateData::GetSplineFacingPointY()
{
    return splineFacingPointY;
}

float UpdateData::GetSplineFacingPointZ()
{
    return splineFacingPointZ;
}

int32 UpdateData::GetSplineTimePassed()
{
    return splineTimePassed;
}

int32 UpdateData::GetSplineDuration()
{
    return splineDuration;
}

uint32 UpdateData::GetSplineID()
{
    return splineId;
}

float UpdateData::GetSplineVerticalAcceleration()
{
    return splineVerticalAcceleration;
}

int UpdateData::GetSplineEffectStartTime()
{
    return splineEffectStartTime;
}

Vector3 UpdateData::GetSplinePoint(int index)
{
    return splinePoints[index];
}

uint32 UpdateData::GetSplinePointCount()
{
    return splinePoints.size();
}

SplineEvaluationMode UpdateData::GetSplineEvaluationMode()
{
    return splineEvaluationMode;
}

float UpdateData::GetSplineEndpointX()
{
    return splineEndpointX;
}

float UpdateData::GetSplineEndpointY()
{
    return splineEndpointY;
}

float UpdateData::GetSplineEndpointZ()
{
    return splineEndpointZ;
}

uint64 UpdateData::GetTransportGUID()
{
    return transportGuid;
}

float UpdateData::GetX()
{
    return x;
}

float UpdateData::GetY()
{
    return y;
}

float UpdateData::GetZ()
{
    return z;
}

float UpdateData::GetTransportOffsetX()
{
    return transportOffsetX;
}

float UpdateData::GetTransportOffsetY()
{
    return transportOffsetY;
}

float UpdateData::GetTransportOffsetZ()
{
    return transportOffsetZ;
}

float UpdateData::GetO()
{
    return o;
}

float UpdateData::GetCorpseOrientation()
{
    return corpseOrientation;
}

uint32 UpdateData::GetLowGUID()
{
    return lowGuid;
}

uint64 UpdateData::GetTargetGUID()
{
    return targetGuid;
}

uint32 UpdateData::GetTransportTimer()
{
    return transportTimer;
}

uint32 UpdateData::GetVehicleID()
{
    return vehicleID;
}

float UpdateData::GetVehicleOrientation()
{
    return vehicleOrientation;
}

uint64 UpdateData::GetGORotation()
{
    return goRotation;
}

std::map<int32, uint32>& UpdateData::GetUpdateFields()
{
    return updateFields;
}

uint32 UpdateData::GetUpdateField(int32 field)
{
    return updateFields[field];
}

bool UpdateData::HasUpdateField(int32 field)
{
    return updateFields.find(field) != updateFields.end();
}

uint64 UpdateData::GetOutOfRangeGUID(uint32 index)
{
    return outOfRangeGuids[index];
}

uint32 UpdateData::OutOfRangeGUIDCount()
{
    return outOfRangeGuids.size();
}

UpdateDataPacket UpdateDataPacket::Read(WorldPacket& packet)
{
    UpdateDataPacket data;
    uint32 blockCount = packet.ReadUInt32();
    data.Entries.reserve(blockCount);
    for (uint32 i = 0; i < blockCount; ++i)
    {
        data.Entries.push_back(UpdateData::Read(packet));
    }
    return data;
}

int inflateBuf(const void* src, int srcLen, void* dst, int dstLen) {
    z_stream strm = { 0 };
    strm.total_in = strm.avail_in = srcLen;
    strm.total_out = strm.avail_out = dstLen;
    strm.next_in = (Bytef*)src;
    strm.next_out = (Bytef*)dst;

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;

    int err = -1;
    int ret = -1;

    err = inflateInit2(&strm, (15 + 32)); //15 window bits, and the +32 tells zlib to to detect if using gzip or zlib
    if (err == Z_OK) {
        err = inflate(&strm, Z_FINISH);
        if (err == Z_STREAM_END) {
            ret = strm.total_out;
        }
        else {
            inflateEnd(&strm);
            return err;
        }
    }
    else {
        inflateEnd(&strm);
        return err;
    }

    inflateEnd(&strm);
    return ret;
}

UpdateDataPacket UpdateDataPacket::ReadCompressed(WorldPacket& packet)
{
    // todo: this is a lot of copies
    uint32 decompressedSize = packet.ReadUInt32();
    std::vector<uint8_t> compressed = packet.ReadBytes(packet.GetPayloadSize() - 4);
    std::vector<uint8_t> decompressed(decompressedSize);
    int res = inflateBuf(compressed.data(), compressed.size(), decompressed.data(), decompressed.size());
    switch (res)
    {
    case Z_BUF_ERROR:
        throw std::runtime_error("zlib output buffer too small");
        break;
    case Z_MEM_ERROR:
        throw std::runtime_error("zlib not enough memory");
        break;
    case Z_DATA_ERROR:
        throw std::runtime_error("zlib received corrupted data");
        break;
    }
    WorldPacket decompressedPacket(Opcodes::SMSG_COMPRESSED_UPDATE_OBJECT, decompressed.size());
    decompressedPacket.WriteBytes(decompressed);
    return Read(decompressedPacket);
}

uint32 UpdateDataPacket::EntryCount()
{
    return Entries.size();
}

UpdateData& UpdateDataPacket::GetEntry(uint32 entry)
{
    return Entries[entry];
}

void UpdateData::Register(sol::state& state)
{
    auto LVector3 = state.new_usertype<Vector3>("Vector3");
    LVector3.set_function("GetX", &Vector3::GetX);
    LVector3.set_function("GetY", &Vector3::GetY);
    LVector3.set_function("GetZ", &Vector3::GetZ);

    auto LUpdateData = state.new_usertype<UpdateData>("UpdateData");
    LUpdateData.set_function("GetUpdateType", &UpdateData::GetUpdateType);
    LUpdateData.set_function("GetGUID", &UpdateData::GetGUID);
    LUpdateData.set_function("GetTypeID", &UpdateData::GetTypeID);
    LUpdateData.set_function("GetUpdateFlags", &UpdateData::GetUpdateFlags);
    LUpdateData.set_function("GetMovement", &UpdateData::GetMovement);
    LUpdateData.set_function("GetWalkSpeed", &UpdateData::GetWalkSpeed);
    LUpdateData.set_function("GetRunSpeed", &UpdateData::GetRunSpeed);
    LUpdateData.set_function("GetRunBackSpeed", &UpdateData::GetRunBackSpeed);
    LUpdateData.set_function("GetSwimSpeed", &UpdateData::GetSwimSpeed);
    LUpdateData.set_function("GetSwimBackSpeed", &UpdateData::GetSwimBackSpeed);
    LUpdateData.set_function("GetFlightSpeed", &UpdateData::GetFlightSpeed);
    LUpdateData.set_function("GetFlightBackSpeed", &UpdateData::GetFlightBackSpeed);
    LUpdateData.set_function("GetTurnRate", &UpdateData::GetTurnRate);
    LUpdateData.set_function("GetPitchRate", &UpdateData::GetPitchRate);
    LUpdateData.set_function("GetSplineFlags", &UpdateData::GetSplineFlags);
    LUpdateData.set_function("GetSplineFacingAngle", &UpdateData::GetSplineFacingAngle);
    LUpdateData.set_function("GetSplineFacingTargetGUID", &UpdateData::GetSplineFacingTargetGUID);
    LUpdateData.set_function("GetSplineFacingPointX", &UpdateData::GetSplineFacingPointX);
    LUpdateData.set_function("GetSplineFacingPointY", &UpdateData::GetSplineFacingPointY);
    LUpdateData.set_function("GetSplineFacingPointZ", &UpdateData::GetSplineFacingPointZ);
    LUpdateData.set_function("GetSplineTimePassed", &UpdateData::GetSplineTimePassed);
    LUpdateData.set_function("GetSplineDuration", &UpdateData::GetSplineDuration);
    LUpdateData.set_function("GetSplineID", &UpdateData::GetSplineID);
    LUpdateData.set_function("GetSplineVerticalAcceleration", &UpdateData::GetSplineVerticalAcceleration);
    LUpdateData.set_function("GetSplineEffectStartTime", &UpdateData::GetSplineEffectStartTime);
    LUpdateData.set_function("GetSplinePoint", &UpdateData::GetSplinePoint);
    LUpdateData.set_function("GetSplinePointCount", &UpdateData::GetSplinePointCount);
    LUpdateData.set_function("GetSplineEvaluationMode", &UpdateData::GetSplineEvaluationMode);
    LUpdateData.set_function("GetSplineEndpointX", &UpdateData::GetSplineEndpointX);
    LUpdateData.set_function("GetSplineEndpointY", &UpdateData::GetSplineEndpointY);
    LUpdateData.set_function("GetSplineEndpointZ", &UpdateData::GetSplineEndpointZ);
    LUpdateData.set_function("GetTransportGUID", &UpdateData::GetTransportGUID);
    LUpdateData.set_function("GetX", &UpdateData::GetX);
    LUpdateData.set_function("GetY", &UpdateData::GetY);
    LUpdateData.set_function("GetZ", &UpdateData::GetZ);
    LUpdateData.set_function("GetTransportOffsetX", &UpdateData::GetTransportOffsetX);
    LUpdateData.set_function("GetTransportOffsetY", &UpdateData::GetTransportOffsetY);
    LUpdateData.set_function("GetTransportOffsetZ", &UpdateData::GetTransportOffsetZ);
    LUpdateData.set_function("GetO", &UpdateData::GetO);
    LUpdateData.set_function("GetCorpseOrientation", &UpdateData::GetCorpseOrientation);
    LUpdateData.set_function("GetLowGUID", &UpdateData::GetLowGUID);
    LUpdateData.set_function("GetTargetGUID", &UpdateData::GetTargetGUID);
    LUpdateData.set_function("GetTransportTimer", &UpdateData::GetTransportTimer);
    LUpdateData.set_function("GetVehicleID", &UpdateData::GetVehicleID);
    LUpdateData.set_function("GetVehicleOrientation", &UpdateData::GetVehicleOrientation);
    LUpdateData.set_function("GetGORotation", &UpdateData::GetGORotation);
    LUpdateData.set_function("GetUpdateFields", [](UpdateData& data) { return sol::as_table(data.GetUpdateFields()); });
    LUpdateData.set_function("GetUpdateField", &UpdateData::GetUpdateField);
    LUpdateData.set_function("HasUpdateField", &UpdateData::HasUpdateField);
    LUpdateData.set_function("GetOutOfRangeGUID", &UpdateData::GetOutOfRangeGUID);
    LUpdateData.set_function("OutOfRangeGUIDCount", &UpdateData::OutOfRangeGUIDCount);

    auto LUpdateDataPacket = state.new_usertype<UpdateDataPacket>("UpdateDataPacket");
    LUpdateDataPacket.set_function("EntryCount", &UpdateDataPacket::EntryCount);
    LUpdateDataPacket.set_function("GetEntry", &UpdateDataPacket::GetEntry);
}
