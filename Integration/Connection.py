import asyncio
from bleak import BleakClient

CHARACTERISTIC_UUID = "68342c53-ac8f-48af-bb4d-7a55375c98a5"  # Same as in ESP32

# Read the database file of tools
def load_tool_database(filename="Integration/tool_database.txt"):
    tools = {}
    with open(filename, "r") as file:
        for line in file:
            if line.strip():
                name, mac = line.strip().split()
                tools[name] = mac
    return tools


async def send_buzz_command(tool_name):
    tools = load_tool_database()

    if tool_name not in tools:
        return False

    selected_mac = tools[tool_name] #get mac address of tool
    try:
        async with BleakClient(selected_mac, timeout=10.0) as client:
            await client.write_gatt_char(CHARACTERISTIC_UUID, b"buzz") #send command, value = "buzz"
            return True
    except Exception as e:
        print(f"Connection failed: {e}")
        return False

async def main():
    tools = load_tool_database()
    if not tools:
        print("No tools found in tool_database.txt")
        return

    search = input("Enter exact tool name to ping (e.g., Tool_05_digitalmultimeter): ").strip()
    if search not in tools:
        print(f"'{search}' not found in the database.")
        return

    selected_mac = tools[search]
    print(f"\nConnecting to {search} ({selected_mac})...")

    try:
        async with BleakClient(selected_mac, timeout=10.0) as client:
            print("Connected. Sending buzz command...")
            await client.write_gatt_char(CHARACTERISTIC_UUID, b"buzz")
            print("Buzz command sent to", search)
    except Exception as e:
        print(f"Connection failed: {e}")

# ## Uncomment to run Connection.py standalone
# asyncio.run(main())