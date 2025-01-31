
function Load_Packet_Type(file_path)
    local Types = {}
    local found_enum_name = nil
    local nextCnt = 0
    for line in io.lines(file_path) do
        local enum_name = line:match("^%s*enum PacketType.*")
        if enum_name then
            found_enum_name = {}
        elseif found_enum_name then
            -- enum parshing
            local name, value = line:match("^%s*(%w+)%s*=%s*(%d+)%s*[,}]?")
            if name and value then
                Types[name] = tonumber(value)
                nextCnt = value + 1
            else
                name = line:match("^%s*(%w+)%s*[,}]?")
                if name then
                    Types[name] = nextCnt
                    nextCnt = nextCnt+1
                end
            end

            -- enum ends
            if line:find("}") then
                return Types
            end
        end
    end
end

PacketType = Load_Packet_Type("../DiscordAttendance/Packets.h")

function ProcessPacketInLua(packet)
    -- lua starts from 1
    local type = string.byte(packet,2)
    if type == PacketType["TEST"] then
        return "TEST PACKET"
    elseif type == PacketType["HELP"] then
        return "HELP PACKET"
    elseif type == PacketType["HELLO"] then
        return "HE1LLO PACKET"
    elseif type == PacketType["BYE"] then
        return "BYE PACKET"
    elseif type == PacketType["FUNNY"] then
        return "FUNNY PACKET"
    end
    return "FAIL"
end

function echo(packet)
    return string.byte(packet,1)
end