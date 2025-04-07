from flask import Flask, request, jsonify
from flask_cors import CORS
from Integration.Connection import send_buzz_command

app = Flask(__name__)
CORS(app)

@app.route('/find_tool', methods=['POST'])
def find_tool():
    data = request.get_json()
    tool_name = data.get("tool")

    if not tool_name:
        return jsonify({"error": "No tool name provided"}), 400

    try:
        print(f"🔗 Buzzing {tool_name} via Bluetooth...")
        send_buzz_command(tool_name)
        print(f"✅ Success.")
        return jsonify({"message": f"{tool_name} buzzed!"}), 200
    except Exception as e:
        print(f"❌ Failed to buzz {tool_name}: {e}")
        return jsonify({"error": f"Failed to buzz {tool_name}"}), 500

if __name__ == '__main__':
    app.run(host="0.0.0.0", port=5000, debug=True)