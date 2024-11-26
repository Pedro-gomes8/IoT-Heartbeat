from flask import Flask, jsonify
from flask_cors import CORS

import sqlite3

app = Flask(__name__)
# Proper CORS setup for specific origins
CORS(app)


@app.route("/heartbeat-history", methods=["GET"])
def get_heartbeat_history():
    conn = sqlite3.connect("heartbeat.db")
    c = conn.cursor()
    c.execute("SELECT timestamp, value FROM heartbeat ORDER BY timestamp DESC LIMIT 20")
    data = c.fetchall()
    conn.close()
    formatted_data = [
        {"date": row[0].split(" ")[1], "heartbeat": row[1]} for row in data
    ]
    return jsonify(formatted_data)


@app.after_request
def add_cors_headers(response):
    print(response.headers)
    return response


@app.route("/heartbeat", methods=["GET"])
def get_heartbeat():
    conn = sqlite3.connect("heartbeat.db")
    c = conn.cursor()
    c.execute("SELECT timestamp, value FROM heartbeat ORDER BY timestamp DESC LIMIT 1")
    data = c.fetchall()
    conn.close()
    if data:
        formatted_data = {"date": data[0][0], "heartbeat": data[0][1]}
    else:
        formatted_data = {"error": "No data available"}
    return jsonify(formatted_data)


if __name__ == "__main__":
    app.run()
