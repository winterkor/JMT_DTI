import asyncio
from Integration.Connection import send_buzz_command

async def test():
    success = await send_buzz_command("Tool_05_digitalmultimeter")
    print("Success:", success)

asyncio.run(test())