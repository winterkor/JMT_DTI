# Scan script to collect MACs
import asyncio
from bleak import BleakScanner

async def main():
    print("Scanning for nearby tools...")
    devices = await BleakScanner.discover(timeout=20.0)

    with open("tool_database.txt", "w") as f:
        for d in devices:
            if d.name and d.name.startswith("Tool_"):
                print(f"{d.name} - {d.address}")
                f.write(f"{d.name} {d.address}\n")

asyncio.run(main())