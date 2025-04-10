### Sets up flask server to communicate with BLE tools 

from flask import Flask, request, jsonify
from flask_cors import CORS
import asyncio
from Integration.Connection import send_buzz_command  # Import function to connect and buzz tool

# Initialise Flask app and allows cross-origin requests 
app = Flask(__name__)
CORS(app) # to let web apps from different domains interact with Flask server

# Create new asyncio event loop to handle asynchronous tasks
loop = asyncio.new_event_loop()
asyncio.set_event_loop(loop)

# Define API endpoint that listens to POST requests
@app.route('/find_tool', methods=['POST'])
# async because it calls an async BLE function
async def find_tool():
    data = request.json
    tool_name = data.get("tool") 

    if not tool_name:
        return jsonify({"error": "No tool name provided"}), 400

    try:
        print(f"Received request to find: {tool_name}")

        # run async function inside the global event loop
        success = loop.run_until_complete(send_buzz_command(tool_name))
        if success: 
            return jsonify({"message": f"{tool_name} buzzed successfully"}), 200
        else: 
            return jsonify({"error": "Failed to buzz tool"}), 500

    except Exception as e:
        print(f"Error: {e}")
        return jsonify({"error": str(e)}), 500

if __name__ == "__main__":
    app.run(debug=True)