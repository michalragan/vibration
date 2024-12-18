from flask import Flask, request, jsonify
from flask_socketio import SocketIO, emit
from pymongo import MongoClient
from datetime import datetime, timezone

app = Flask(__name__)
socketio = SocketIO(app, cors_allowed_origins="*")

client = MongoClient("mongodb://mongo:27017/")
db = client["vibration_db"]
collection = db["vibrations"]

@app.route('/', methods=['GET'])
def home():
    return "Flask backend works properly!", 200

@app.route('/vibrations', methods=['POST'])
def receive_data():
    try:
        data = request.json
        if not data or "value" not in data:
            return jsonify({"error": "Field 'value' is required"}), 400

        current_time = datetime.now(timezone.utc)
        data_to_insert = {"value": data["value"], "timestamp": current_time}
        result = collection.insert_one(data_to_insert)
        data_to_insert["_id"] = str(result.inserted_id)

        if collection.count_documents({}) >= 100:
            collection.delete_many({})

        latest_entries = list(collection.find().sort("timestamp", -1))
        payload = [
            {
                "_id": str(entry["_id"]),
                "value": entry["value"],
                "timestamp": entry["timestamp"].strftime("%H:%M:%S") if isinstance(entry["timestamp"], datetime) else entry["timestamp"]
            }
            for entry in reversed(latest_entries)
        ]

        socketio.emit('vibration_update', payload)

        return jsonify({"status": "success", "data": data_to_insert}), 200

    except Exception as e:
        print(f"Server error: {e}")
        return jsonify({"error": "Internal Server Error", "details": str(e)}), 500

    
if __name__ == '__main__':
    socketio.run(app, host="0.0.0.0", port=5001)
