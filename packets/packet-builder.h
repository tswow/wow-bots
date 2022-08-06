#pragma once

#include "../src/Bots/BotOpcodes.h"

#include <map>
#include <string>
#include <memory>
#include <sstream>
#include <variant>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <optional>

namespace fs = std::filesystem;

enum class StringVariableType
{
    C_STRING,
    STATIC,
    SIZED
};

enum class PacketType
{
    WRITE,
    READ,
    BOTH
};

bool IsWrite(PacketType type)
{
    return type == PacketType::WRITE || type == PacketType::BOTH;
}

bool IsRead(PacketType type)
{
    return type == PacketType::READ || type == PacketType::BOTH;
}

struct CString {};
struct StaticString
{
    uint32_t size;
};
struct SizedString {
    std::string m_typeName;
};

class PacketBuilder
{
public:

    using TypeReferenceKind = std::variant<PacketBuilder*,CString,StaticString,SizedString,std::string>;
    enum class VariableKind
    {
        ARRAY,
        VECTOR,
        INDIVIDUAL
    };

    struct BaseTypeRef
    {
        virtual ~BaseTypeRef() {};
        virtual bool IsVariable() = 0;
        virtual bool IsComplex() = 0;
        virtual void RenderWrite(std::stringstream& stream, std::string const& packetName) = 0;
        virtual void RenderRead(std::stringstream& stream, std::string const& targetName, std::string const& packetName) = 0;
    };

    enum class TypeNameContext
    {
        TYPESCRIPT,
        CXX
    };

    struct VarTypeRef : public BaseTypeRef
    {
        TypeReferenceKind m_varType;
        std::string m_name;
        VarTypeRef(TypeReferenceKind const& varType, std::string const& name)
            : m_varType(varType)
            , m_name(name)
        {}

        bool IsVariable() override { return true; }

        bool IsVarComplex()
        {
            return IsStringType() || (std::holds_alternative<PacketBuilder*>(m_varType) && std::get<PacketBuilder*>(m_varType)->IsComplex());
        }

        virtual void RenderTypeScriptDeclaration(std::stringstream& stream, std::string const& ownerType, PacketType type) = 0;
        virtual void RenderAccessorDecl(std::stringstream& stream, std::string const& ownerType, PacketType type) = 0;
        virtual void RenderAccessorImpl(std::stringstream& stream, std::string const& ownerType, PacketType type) = 0;
        virtual void RenderLuaRegistry(std::stringstream& stream, std::string const& luaTarget, std::string const& ownerType, PacketType type) = 0;

        void RenderVariableDeclaration(std::stringstream& stream)
        {
            stream << GetFullTypeName(TypeNameContext::CXX) << " " << m_name << ";";
        }

        bool IsStringType()
        {
            return std::holds_alternative<CString>(m_varType)
                || std::holds_alternative<SizedString>(m_varType)
                || std::holds_alternative<StaticString>(m_varType);
        }

        std::string GetArgumentType()
        {
            if (IsStringType())
            {
                return "std::string const&";
            }
            if (std::holds_alternative<PacketBuilder*>(m_varType))
            {
                return GetBasicTypeName(TypeNameContext::CXX) + "&";
            }
            return GetBasicTypeName(TypeNameContext::CXX);
        }

        std::string GetReturnType()
        {
            if (std::holds_alternative<PacketBuilder*>(m_varType))
            {
                return GetBasicTypeName(TypeNameContext::CXX) + "&";
            }
            return GetBasicTypeName(TypeNameContext::CXX);
        }

        std::string GetBasicTypeName(TypeNameContext ctx)
        {
            if (std::holds_alternative<std::string>(m_varType))
            {
                return std::get<std::string>(m_varType);
            }
            else if (std::holds_alternative<PacketBuilder*>(m_varType))
            {
                return std::get<PacketBuilder*>(m_varType)->m_name;
            }
            else if (IsStringType())
            {
                switch (ctx)
                {
                case TypeNameContext::TYPESCRIPT:
                    return "string";
                case TypeNameContext::CXX:
                    return "std::string";
                default:
                    throw std::runtime_error("Invalid TypeNameContext " + std::to_string(int(ctx)));
                }
            }
            else
            {
                throw std::runtime_error("Error getting type declaration for " + m_name);
            }
        }

        void RenderVarWrite(std::stringstream& stream, std::string const& packetName, std::string const& varName)
        {
            if (std::holds_alternative<PacketBuilder*>(m_varType))
            {
                stream << varName << ".Write(" << packetName << ");";
            }
            else if (std::holds_alternative<std::string>(m_varType))
            {
                stream << packetName << ".Write<" << std::get<std::string>(m_varType) << ">(" << varName << ");";
            }
            else if (std::holds_alternative<CString>(m_varType))
            {
                stream << packetName << ".WriteCString(" << varName << ");";
            }
            else if (std::holds_alternative<StaticString>(m_varType))
            {
                stream << packetName << ".WriteString(" << varName << ");";
            }
            else if (std::holds_alternative<SizedString>(m_varType))
            {
                SizedString str = std::get<SizedString>(m_varType);
                stream << packetName << ".Write<" << str.m_typeName << ">(" << varName << ").size();" << packetName << ".WriteString(" << varName << ");";
            }
        }

        void RenderVarRead(std::stringstream& stream, std::string const& packetName, std::string const& varName)
        {
            if (std::holds_alternative<PacketBuilder*>(m_varType))
            {
                stream << varName << " = " << std::get<PacketBuilder*>(m_varType)->m_name << "::Read(" << packetName << ");";
            }
            else if (std::holds_alternative<std::string>(m_varType))
            {
                stream << varName << " = " << packetName << ".Read<" << std::get<std::string>(m_varType) << ">();";
            }
            else if (std::holds_alternative<CString>(m_varType))
            {
                stream << varName << " = " << packetName << ".ReadCString();";
            }
            else if (std::holds_alternative<StaticString>(m_varType))
            {
                stream << varName << " = " << packetName << ".ReadString(" << std::get<StaticString>(m_varType).size << ");";
            }
            else if (std::holds_alternative<SizedString>(m_varType))
            {
                stream << varName << " = " << packetName << ".ReadString("
                    << packetName << ".Read<" << std::get<SizedString>(m_varType).m_typeName << ">());";
            }
        }

        virtual std::string GetFullTypeName(TypeNameContext ctx) = 0;
    };

    struct SingleVarTypeRef : public VarTypeRef
    {
        SingleVarTypeRef(TypeReferenceKind const& kind, std::string const& typeName)
            : VarTypeRef(kind,typeName)
        {}

        std::string GetFullTypeName(TypeNameContext ctx) override
        {
            return GetBasicTypeName(ctx);
        }

        void RenderWrite(std::stringstream& stream, std::string const& packetName) override
        {
            RenderVarWrite(stream, packetName, m_name);
            stream << ";";
        }

        void RenderRead(std::stringstream& stream, std::string const& targetName, std::string const& packetName) override
        {
            RenderVarRead(stream, packetName, targetName + "." + m_name);
            stream << ";";
        }

        void RenderTypeScriptDeclaration(std::stringstream& stream, std::string const& ownerType, PacketType type) override
        {
            if (IsRead(type))
            {
                stream << "    Get" << m_name << "(): " << GetBasicTypeName(TypeNameContext::TYPESCRIPT) << ";\n";
            }

            if (IsWrite(type))
            {
                stream << "    Set" << m_name << "(value: " << GetBasicTypeName(TypeNameContext::TYPESCRIPT) << "): " << ownerType << ";\n";
            }
        }

        void RenderLuaRegistry(std::stringstream& stream, std::string const& luaTarget, std::string const& ownerType, PacketType type)
        {
            if (IsRead(type))
            {
                stream << "    " << luaTarget << ".set_function(\"Get" << m_name << "\", &" << ownerType << "::Get" << m_name << ");\n";
            }

            if (IsWrite(type))
            {
                stream << "    " << luaTarget << ".set_function(\"Set" << m_name << "\", &" << ownerType << "::Set" << m_name << ");\n";
            }
        }

        void RenderAccessorDecl(std::stringstream& stream, std::string const& ownerType, PacketType type) override
        {
            if (IsRead(type))
            {
                stream << "    " << GetReturnType() << " Get" << m_name << "();\n";
            }

            if (IsWrite(type))
            {
                stream << "    " << ownerType << "& Set" << m_name << "(" << GetArgumentType() << " value);";
            }
        }

        void RenderAccessorImpl(std::stringstream& stream, std::string const& ownerType, PacketType type) override
        {
            if (IsRead(type))
            {
                stream << GetReturnType() << " " << ownerType << "::Get" << m_name << "()\n";
                stream << "{\n";
                stream << "    return " << m_name << ";\n";
                stream << "}\n";
            }

            if (IsWrite(type))
            {
                stream << ownerType << "& " << ownerType << "::Set" << m_name << "(" << GetArgumentType() << " value)";
                stream << "{\n";
                stream << "    " << m_name << " = value;\n";
                stream << "    return *this;\n";
                stream << "}\n";
            }
        }

        bool IsComplex() override
        {
            return IsVarComplex();
        }
    };

    struct ArrayTypeRef : public VarTypeRef
    {
        uint32_t m_size;

        ArrayTypeRef(TypeReferenceKind const& kind, std::string const& typeName, uint32_t size)
            : VarTypeRef(kind, typeName)
            , m_size(size)
        {}

        void RenderTypeScriptDeclaration(std::stringstream& stream, std::string const& ownerType, PacketType type) override
        {
            if (IsRead(type))
            {
                stream << "    Get" << m_name << "(index: number): " << GetBasicTypeName(TypeNameContext::TYPESCRIPT) << ";\n";
            }

            if (IsWrite(type))
            {
                stream << "    Set" << m_name << "(index: number, value: " << GetBasicTypeName(TypeNameContext::TYPESCRIPT) << "): " << ownerType << ";\n";
            }
            stream << "    " << m_name << "Count(): number;\n";
        }

        void RenderLuaRegistry(std::stringstream& stream, std::string const& luaTarget, std::string const& ownerType, PacketType type)
        {
            if (IsRead(type))
            {
                stream << "    " << luaTarget << ".set_function(\"Get" << m_name << "\", &" << ownerType << "::Get" << m_name << ");\n";
            }

            if (IsWrite(type))
            {
                stream << "    " << luaTarget << ".set_function(\"Set" << m_name << "\", &" << ownerType << "::Set" << m_name << ");\n";
            }
            stream << "    " << luaTarget << ".set_function(\"" << m_name << "Count\", &" << ownerType << "::" << m_name << "Count);\n";
        }

        void RenderAccessorDecl(std::stringstream& stream, std::string const& ownerType, PacketType type) override
        {
            if (IsRead(type))
            {
                stream << "    " << GetReturnType() << " Get" << m_name << "(uint32_t index);\n";
            }

            if (IsWrite(type))
            {
                stream << "    " << ownerType << "& Set" << m_name << "(uint32_t index, " << GetArgumentType() << ");\n";
            }
            stream << "    double " << m_name << "Count();\n";
        }

        void RenderAccessorImpl(std::stringstream& stream, std::string const& ownerType, PacketType type) override
        {
            if (IsRead(type))
            {
                stream << GetReturnType() << " " << ownerType << "::Get" << m_name << "(uint32_t index)";
                stream << "{\n";
                stream << "    return " << m_name << "[index];\n";
                stream << "}\n";
            }

            if (IsWrite(type))
            {
                stream << ownerType << "& " << ownerType << "::Set" << m_name << "(uint32_t index, " << GetArgumentType() << " value)";
                stream << "{\n";
                stream << "    " << m_name << "[index] = value;\n";
                stream << "    return *this;\n";
                stream << "}\n";
            }
            stream << "double " << ownerType << "::" << m_name << "Count() { return " << m_name << ".size(); }\n";
        }

        void RenderWrite(std::stringstream& stream, std::string const& packetName) override
        {
            // TODO: write non-complex types in a single write
            stream << "for(int i=0; i < " << m_size << "; ++i)\n";
            stream << "    {\n";
            stream << "        ";
            RenderVarWrite(stream, packetName, m_name + "[i]");
            stream << "\n    }";
        }
        void RenderRead(std::stringstream& stream, std::string const& targetName, std::string const& packetName) override
        {
            stream << "for(int i=0;i < " << m_size << "; ++i)\n";
            stream << "    {\n";
            stream << "        ";
            RenderVarRead(stream, packetName, targetName + "." + m_name + "[i]");
            stream << "\n    }";
        }

        std::string GetFullTypeName(TypeNameContext ctx) override
        {
            switch (ctx)
            {
            case TypeNameContext::CXX:
                return "std::array<" + GetBasicTypeName(ctx) + "," + std::to_string(m_size) + ">";
            case TypeNameContext::TYPESCRIPT:
                return GetBasicTypeName(ctx) + "[]";
            default:
                throw std::runtime_error("Invalid TypeNameContext " + std::to_string(int(ctx)));
            }
        }

        bool IsComplex() override
        {
            return IsVarComplex();
        }
    };

    struct VectorTypeRef: public VarTypeRef
    {
        std::string m_sizeType;

        VectorTypeRef(TypeReferenceKind const& kind, std::string const& typeName, std::string const& sizeType)
            : VarTypeRef(kind, typeName)
            , m_sizeType(sizeType)
        {}

        void RenderTypeScriptDeclaration(std::stringstream& stream, std::string const& ownerType, PacketType type) override
        {
            if (IsRead(type))
            {
                stream << "    Get" << m_name << "(index: number): " << GetBasicTypeName(TypeNameContext::TYPESCRIPT) << ";\n";
            }

            if (IsWrite(type))
            {
                stream << "    Add" << m_name << "(value: " << GetBasicTypeName(TypeNameContext::TYPESCRIPT) << "): " << ownerType << ";\n";
            }
            stream << "    " << m_name << "Count(): number;\n";
        }

        void RenderLuaRegistry(std::stringstream& stream, std::string const& luaTarget, std::string const& ownerType, PacketType type)
        {
            if (IsRead(type))
            {
                stream << "    " << luaTarget << ".set_function(\"Get" << m_name << "\", &" << ownerType << "::Get" << m_name << ");\n";
            }

            if (IsWrite(type))
            {
                stream << "    " << luaTarget << ".set_function(\"Add" << m_name << "\", &" << ownerType << "::Add" << m_name << ");\n";
            }
            stream << "    " << luaTarget << ".set_function(\"" << m_name << "Count\", &" << ownerType << "::" << m_name << "Count);\n";
        }

        void RenderAccessorDecl(std::stringstream& stream, std::string const& ownerType, PacketType type) override
        {
            if (IsRead(type))
            {
                stream << "    " << GetReturnType() << " Get" << m_name << "(uint32_t index);\n";
            }

            if (IsWrite(type))
            {
                stream << "    " << ownerType << "& Add" << m_name << "(" << GetArgumentType() << ");\n";
            }

            stream << "double " << m_name << "Count();\n";
        }

        void RenderAccessorImpl(std::stringstream& stream, std::string const& ownerType, PacketType type) override
        {
            if (IsRead(type))
            {
                stream << GetReturnType() << " " << ownerType << "::Get" << m_name << "(uint32_t index)";
                stream << "{\n";
                stream << "    return " << m_name << "[index];\n";
                stream << "}\n";
            }

            if (IsWrite(type))
            {
                stream << ownerType << "& " << ownerType << "::Add" << m_name << "(" << GetArgumentType() << " value)";
                stream << "{\n";
                stream << "    " << m_name << ".push_back(value);\n";
                stream << "    return *this;\n";
                stream << "}\n";
            }

            stream << "double " << ownerType << "::" << m_name << "Count() { return " << m_name << ".size(); }\n";
        }

        void RenderWrite(std::stringstream& stream, std::string const& packetName) override
        {
            // TODO: write non-complex types in a single write
            stream << "    for(int i=0; i < " + m_name + ".size(); ++i)";
            stream << "    {\n";
            stream << "        ";
            RenderVarWrite(stream, packetName, m_name + "[i]");
            stream << "\n    }";
        }
        void RenderRead(std::stringstream& stream, std::string const& targetName, std::string const& packetName) override
        {
            stream << "    " << m_sizeType << " _size = " << packetName << ".Read<" << m_sizeType << ">();\n";
            stream << "    " << targetName << "." << m_name << ".resize(_size);\n";
            stream << "    for(int i=0;i < _size ; ++i)\n";
            stream << "    {\n";
            stream << "        ";
            RenderVarRead(stream, packetName, targetName + "." + m_name + "[i]");
            stream << "\n    }";
        }

        std::string GetFullTypeName(TypeNameContext ctx) override
        {
            switch (ctx)
            {
            case TypeNameContext::CXX:
                return "std::vector<" + GetBasicTypeName(ctx) + ">";
            case TypeNameContext::TYPESCRIPT:
                return GetBasicTypeName(ctx) + "[]";
            default:
                throw std::runtime_error("Invalid TypeNameContext " + std::to_string(int(ctx)));
            }
        }

        bool IsComplex() override
        {
            return false;
        }
    };

    struct PaddingType : public BaseTypeRef
    {
        size_t m_amount;
        bool IsVariable() override { return false; }
        bool IsComplex() override { return false; };
        void RenderWrite(std::stringstream& stream, std::string const& packetName) override
        {
            stream << packetName << ".WritePadding(" << m_amount << ");";
        }

        void RenderRead(std::stringstream& stream, std::string const&, std::string const& packetName) override
        {
            stream << packetName << ".ReadBytes(" << m_amount << ");";
        }
    };

    PacketBuilder(std::string const& name)
        : m_name(name)
    {}

    PacketBuilder() = default;

    PacketBuilder* SingleField(TypeReferenceKind type, std::string const& name)
    {
        m_references.push_back(std::make_unique<SingleVarTypeRef>(type,name));
        return this;
    }

    PacketBuilder* ArrayField(TypeReferenceKind type, std::string const& name, uint32_t size)
    {
        m_references.push_back(std::make_unique<ArrayTypeRef>(type,name,size));
        return this;
    }

    PacketBuilder* VectorField(TypeReferenceKind type, std::string const& name, std::string const& sizeType)
    {
        m_references.push_back(std::make_unique<VectorTypeRef>(type,name,sizeType));
        return this;
    }

    PacketBuilder* Padding(uint32_t size)
    {
        m_references.push_back(std::make_unique<PaddingType>());
        return this;
    }

    bool IsComplex()
    {
        for (std::unique_ptr<BaseTypeRef>& ref : m_references)
        {
            if (ref->IsComplex())
            {
                return true;
            }
        }
        return false;
    }

    void WriteHeaderFile(std::stringstream & stream)
    {
        stream << "class " << m_name << "\n";
        stream << "{\n";
        stream << "public:\n";
        for (std::unique_ptr<BaseTypeRef>& ref : m_references)
        {
            if (VarTypeRef* var = dynamic_cast<VarTypeRef*>(ref.get()))
            {
                var->RenderAccessorDecl(stream, m_name, m_type);
            }
        }
        stream << "    static void Register(sol::state& state);\n";
        stream << "    static " << m_name << " Read(WorldPacket& packet);\n";
        stream << "    void Write(WorldPacket& packet);\n";
        if (IsWrite(m_type))
        {
            stream << "    static " << m_name << " create();\n";
        }
        if (IsClientPacket())
        {
            stream << "    void Send(Bot& bot);\n";
        }

        stream << "private:\n";
        for (std::unique_ptr<BaseTypeRef>& ref : m_references)
        {
            if (VarTypeRef* var = dynamic_cast<VarTypeRef*>(ref.get()))
            {
                stream << "    ";
                var->RenderVariableDeclaration(stream);
                stream << "\n";
            }
        }
        stream << "};\n";
    }

    void WriteSourceFile(std::stringstream& stream)
    {
        stream << "\n";
        for (std::unique_ptr<BaseTypeRef>& ref : m_references)
        {
            if (VarTypeRef* var = dynamic_cast<VarTypeRef*>(ref.get()))
            {
                stream << "    ";
                var->RenderAccessorImpl(stream, m_name, m_type);
                stream << "\n";
            }
        }

        stream << m_name << " " << m_name << "::Read(WorldPacket& packet)\n";
        stream << "{\n";
        stream << "    " << m_name << " target;\n";
        for (std::unique_ptr<BaseTypeRef>& ref : m_references)
        {
            stream << "    ";
            ref->RenderRead(stream, "target", "packet");
            stream << "\n";
        }
        stream << "    return target;\n";
        stream << "}\n";
        stream << "\n";
        stream << "void " << m_name << "::Write(WorldPacket& packet)\n";
        stream << "{\n";
        for (std::unique_ptr<BaseTypeRef>& ref : m_references)
        {
            stream << "    ";
            ref->RenderWrite(stream, "packet");
            stream << "\n";
        }
        stream << "}\n";

        stream << "void " << m_name << "::Register(sol::state& state)\n";
        stream << "{\n";

        stream << "    auto L" << m_name << " = state.new_usertype<" << m_name << ">(\"" << m_name << "\");\n";

        if (IsWrite(m_type))
        {
            stream << "L" << m_name << ".set_function(\"create\",&"<< m_name << "::create);\n";
        }

        if (IsClientPacket())
        {
            stream << "L" << m_name << ".set_function(\"Send\",&"<< m_name << "::Send);\n";
        }
        for (std::unique_ptr<BaseTypeRef>& ref : m_references)
        {
            if (VarTypeRef* var = dynamic_cast<VarTypeRef*>(ref.get()))
            {
                stream << "    ";
                var->RenderLuaRegistry(stream, "L" + m_name, m_name, m_type);
                stream << "\n";
            }
        }
        stream << "}\n";

        if (IsClientPacket())
        {
            stream << "void " << m_name << "::Send(Bot& bot)\n";
            stream << "{\n";
            // todo: predict size
            stream << "    WorldPacket packet(Opcodes(" << std::to_string(int(m_opcode.value())) << "));\n";
            stream << "    Write(packet);\n";
            stream << "    packet.SendNoWait(bot);\n";
            stream << "}\n";

        }
        if (IsWrite(m_type))
        {
            stream << m_name << " " << m_name << "::create()\n";
            stream << "{\n";
            stream << "    return " << m_name << "();\n";
            stream << "}\n";
        }
    }

    void WriteTypeScriptEvent(std::stringstream& stream)
    {
        if (IsServerPacket())
        {
            stream << "On" << m_name << "(callback: (bot: Bot, packet: " << m_name << ") => void): BotProfile;\n";
        }
    }

    void WriteForwardDeclaration(std::stringstream& stream)
    {
        if (IsServerPacket())
        {
            stream << "class " << m_name << ";\n";
        }
    }

    void WriteHeaderMacro(std::stringstream& stream)
    {
        if (IsServerPacket())
        {
            stream << "    BotProfile On" << m_name << "(std::function<void(Bot&," << m_name << "&)> callback);\\\n";
        }
    }

    void WriteEventLuaMacro(std::stringstream& stream, std::string const& stateName)
    {
        if (IsServerPacket())
        {
            stream << "    " << stateName << ".set_function(\"On" << m_name
                << "\", [](BotProfile& profile, sol::protected_function callback) "
                << "{ return profile.On" << m_name << "([=](Bot& bot, " << m_name << "& packet) { callback(bot,packet);} ); }); \\\n";
        }
    }

    void WriteEventImplementation(std::stringstream& stream)
    {
        if (IsServerPacket())
        {
            stream << "BotProfile BotProfile::On" << m_name << "(std::function<void(Bot&," << m_name << "&)> callback) {";
            stream << " OnWorldPacket(" << int(m_opcode.value()) << ", [=](Bot& bot, WorldPacket& packet) {";
            stream << m_name << " parsed = " << m_name << "::Read(packet) ; callback(bot,parsed); }); return *this; }\n";
        }
    }

    void WriteTypeScriptDeclaration(std::stringstream& stream)
    {
        stream << "declare class " << m_name << "\n";
        stream << "{\n";
        for (std::unique_ptr<BaseTypeRef>& ref : m_references)
        {
            if (VarTypeRef* var = dynamic_cast<VarTypeRef*>(ref.get()))
            {
                var->RenderTypeScriptDeclaration(stream,m_name,m_type);
            }
        }

        if (IsClientPacket())
        {
            stream << "    Send(bot: Bot): void;\n";
        }

        if (IsWrite(m_type))
        {
            stream << "    static create(): " << m_name << ";\n";
        }
        stream << "}\n";
    }

    static void WriteIfDifferent(fs::path path, std::string const& value)
    {
        if (fs::exists(path))
        {
            std::ifstream in(path);
            std::stringstream sstream;
            sstream << in.rdbuf();
            std::string old = sstream.str();
            if (old == value)
            {
                return;
            }
            std::cout << "Regenerating " << path << "\n";
        }
        else
        {
            std::cout << "Generating " << path << "\n";
        }
        std::ofstream out(path);
        out << value;
    }

    static void Generate()
    {
        fs::path generatedRoot("packets.generated");
        if (!fs::exists(generatedRoot))
        {
            fs::create_directories(generatedRoot);
        }

        std::stringstream fwdHeader;
        std::stringstream source;
        std::stringstream tsDeclaration;
        std::stringstream header;
        std::stringstream luaHeader;

        header << "// This file is auto-generated, don't edit it!\n\n";
        header << "#pragma once\n";
        header << "#include \"PacketTypes.h\"\n";
        header << "class WorldPacket;\n";
        header << "class Bot;\n";
        header << "namespace sol { class state; }\n";
        luaHeader << "// This file is auto-generated, don't edit it!\n\n";
        fwdHeader << "// This file is auto-generated, don't edit it!\n\n";
        tsDeclaration << "// This file is auto-generated, don't edit it!\n\n";
        source << "// This file is auto-generated, don't edit it!\n\n";
        source << "#include \"sol/sol.hpp\"\n";
        source << "#include \"Packets.h\"\n";
        source << "#include \"BotPacket.h\"\n";
        source << "#include \"BotProfile.h\"\n";
        source << "#include \"Bot.h\"\n";

        tsDeclaration << "interface BotProfile {\n";
        for (auto& builder : GetBuilders())
        {
            builder->WriteTypeScriptEvent(tsDeclaration);
        }
        tsDeclaration << "}\n";
        tsDeclaration << "\n";

        luaHeader << "namespace sol { class state; }\n";
        luaHeader << "void RegisterPacketLua(sol::state& state);\n";
        luaHeader << "\n";
        luaHeader << "#define LUA_EVENTS(state) \\\n";
        for (auto& builder : GetBuilders())
        {
            builder->WriteEventLuaMacro(luaHeader, "state");
        }

        for (auto& builder : GetBuilders())
        {
            builder->WriteForwardDeclaration(fwdHeader);
        }
        fwdHeader << "\n";
        fwdHeader << "#define PACKET_EVENTS_DECL \\\n";
        for (auto& builder : GetBuilders())
        {
            builder->WriteHeaderMacro(fwdHeader);
        }

        for (auto& builder : GetBuilders())
        {
            builder->WriteHeaderFile(header);
            builder->WriteSourceFile(source);
            builder->WriteEventImplementation(source);
            builder->WriteTypeScriptDeclaration(tsDeclaration);
        }

        source << "void RegisterPacketLua(sol::state& state)\n";
        source << "{\n";
        for (auto& builder : GetBuilders())
        {
            source << "    " << builder->m_name << "::Register(state);\n";
        }
        source << "}\n";

        WriteIfDifferent(generatedRoot / "PacketsFwd.h", fwdHeader.str());
        WriteIfDifferent(generatedRoot / "Packets.ipp", source.str());
        WriteIfDifferent("typescript/packets.global.d.ts", tsDeclaration.str());
        WriteIfDifferent(generatedRoot / "Packets.h", header.str());
        WriteIfDifferent(generatedRoot / "PacketLua.h", luaHeader.str());
    }

    static PacketBuilder* CreatePacket(Opcodes opcode, PacketType type, std::string const& name)
    {
        PacketBuilder* builder = CreateChunk(type, name);
        builder->m_opcode.emplace(opcode);
        return builder;
    }

    static PacketBuilder* CreateChunk(PacketType type, std::string const& name)
    {
        PacketBuilder* builder = GetBuilders().emplace_back(std::make_unique<PacketBuilder>()).get();
        builder->m_name = name;
        builder->m_type = type;
        return builder;
    }

    bool IsClientPacket()
    {
        return IsWrite(m_type) && m_opcode.has_value();
    }

    bool IsServerPacket()
    {
        return IsRead(m_type) && m_opcode.has_value();
    }
private:
    static std::vector<std::unique_ptr<PacketBuilder>>& GetBuilders();
    std::vector<std::unique_ptr<BaseTypeRef>> m_references;
    std::string m_name;
    std::optional<Opcodes> m_opcode;
    PacketType m_type;
};