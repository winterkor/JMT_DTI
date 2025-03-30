import asyncio
from bleak import BleakClient

CHARACTERISTIC_UUID = "68342c53-ac8f-48af-bb4d-7a55375c98a5"  # Same as in ESP32

def load_tool_database(filename="tool_database.txt"):
    tools = {}
    with open(filename, "r") as file:
        for line in file:
            if line.strip():
                name, mac = line.strip().split()
                tools[name] = mac
    return tools

async def main():
    tools = load_tool_database()

    if not tools:
        print("No tools found in tool_database.txt")
        return

    search = input("Enter exact tool name to ping (e.g., Tool_01): ").strip()

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

asyncio.run(main())