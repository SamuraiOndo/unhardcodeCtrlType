import json 
from binary_reader import BinaryReader
import os
import sys

def extractctrltype(file_path):
    f = open(file_path, 'rb')
    reader = BinaryReader(f.read())
    filejson1 = open(file_path + ".json","w",encoding='utf-8')
    count = os.path.getsize(file_path) // 172
    header = {
        "Count": count,
        }
    for i in range(count):
        ctrlName = reader.read_str(32)
        JISString = reader.read_str(32,encoding='cp932')
        Modelname = reader.read_str(32)
        weaponname = reader.read_str(36)
        health = reader.read_uint16()
        unk1 = reader.read_uint16()
        height = reader.read_uint8()
        unk2 = reader.read_uint8()
        team = reader.read_uint8()
        damage = reader.read_uint8()
        unkstring = reader.read_str(32)


        data = {
            "Ctrl Name": ctrlName,
            "Name": JISString,
            "Model Name": Modelname,
            "Weapon Name": weaponname,
            "Health": health,
            "Unk1": unk1,
            "Height": height,
            "Unk2": unk2,
            "Team": team,
            "Damage": damage,
            "Unk String": unkstring
            }
        header.update({i: data})

    filejson1.write(json.dumps(header, indent=4, ensure_ascii=False))
    f.close()
    filejson1.close()

def repackCtrlType(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        data = json.load(f)
    file = open(file_path.replace(".json", ""), 'wb')
    writer = BinaryReader()
    for i in range(data["Count"]):
        writer.write_str_fixed(data[str(i)]["Ctrl Name"], 32)
        writer.write_str_fixed(data[str(i)]["Name"], 32, encoding='cp932')
        writer.write_str_fixed(data[str(i)]["Model Name"], 32)
        writer.write_str_fixed(data[str(i)]["Weapon Name"], 36)
        writer.write_uint16(data[str(i)]["Health"])
        writer.write_uint16(data[str(i)]["Unk1"])
        writer.write_uint8(data[str(i)]["Height"])
        writer.write_uint8(data[str(i)]["Unk2"])
        writer.write_uint8(data[str(i)]["Team"])
        writer.write_uint8(data[str(i)]["Damage"])
        writer.write_str_fixed(data[str(i)]["Unk String"], 32)
    file.write(writer.buffer())
    file.close()

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: extractctrltype.py <file>")
        sys.exit(1)
    if (sys.argv[1].endswith(".json")):
        repackCtrlType(sys.argv[1])
    else:
        extractctrltype(sys.argv[1])