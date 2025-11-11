const express = require("express");
const bodyParser = require("body-parser");
const mqtt = require("mqtt");
const path = require("path");
const db = require("./db");

const app = express();
app.use(bodyParser.json());
app.use(express.static("public")); // serve file HTML dari folder public

// =========================
// KONFIGURASI MQTT
// =========================
const broker = "mqtt://broker.mqttdashboard.com";
const clientId = "server-backend-" + Math.random().toString(16).substr(2, 8);

const topicSuhu = "rido/suhu";
const topicKelembapan = "rido/kelembapan";
const topicLux = "rido/lux";
const topicLed = "rido/led";

const mqttClient = mqtt.connect(broker, { clientId });

// Variabel untuk menyimpan data sementara
let latestData = {
  suhu: null,
  kelembapan: null,
  lux: null,
  lastSaved: 0
};

mqttClient.on("connect", () => {
  console.log("✅ Connected to MQTT Broker");

  mqttClient.subscribe([topicSuhu, topicKelembapan, topicLux], (err) => {
    if (!err) {
      console.log("📡 Subscribed to topics:");
      console.log(`   - ${topicSuhu}`);
      console.log(`   - ${topicKelembapan}`);
      console.log(`   - ${topicLux}`);
    } else {
      console.error("❌ Error subscribing:", err);
    }
  });
});

mqttClient.on("message", (topic, message) => {
  const value = parseFloat(message.toString());
  const now = Date.now();

  if (topic === topicSuhu) latestData.suhu = value;
  if (topic === topicKelembapan) latestData.kelembapan = value;
  if (topic === topicLux) latestData.lux = value;

  console.log(`📥 Topic ${topic} => ${value}`);

  if (
    latestData.suhu !== null &&
    latestData.kelembapan !== null &&
    latestData.lux !== null &&
    now - latestData.lastSaved > 2000
  ) {
    const sql =
      "INSERT INTO data_sensor (suhu, humidity, lux) VALUES (?, ?, ?)";
    db.query(
      sql,
      [latestData.suhu, latestData.kelembapan, latestData.lux],
      (err) => {
        if (err) console.error("❌ DB Error:", err);
        else {
          console.log(
            `✅ Data saved: suhu=${latestData.suhu}, kelembapan=${latestData.kelembapan}, lux=${latestData.lux}`
          );
          latestData.lastSaved = now;
        }
      }
    );
  }
});

// =========================
// ENDPOINT API
// =========================

// Semua data
app.get("/api/data", (req, res) => {
  db.query("SELECT * FROM data_sensor ORDER BY id DESC", (err, results) => {
    if (err) return res.status(500).json({ error: err });
    res.json(results);
  });
});

// Data terbaru saja
app.get("/api/data/latest", (req, res) => {
  db.query("SELECT * FROM data_sensor ORDER BY id DESC LIMIT 1", (err, results) => {
    if (err) return res.status(500).json({ error: err });
    res.json(results[0]);
  });
});

// API kirim perintah LED
app.post("/api/led", (req, res) => {
  const { status } = req.body; // 'on' atau 'off'
  if (status !== "on" && status !== "off") {
    return res.status(400).json({ error: "Invalid LED status" });
  }

  mqttClient.publish(topicLed, status);
  console.log(`💡 LED set to ${status}`);
  res.json({ success: true, message: `LED turned ${status}` });
});

// Serve halaman utama HTML
app.get("/", (req, res) => {
  res.sendFile(path.join(__dirname, "public", "index.html"));
});

// Jalankan server
const PORT = 3000;
app.listen(PORT, () => {
  console.log(`🚀 Server running on http://localhost:${PORT}`);
});
