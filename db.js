const mysql = require("mysql2");

const db = mysql.createConnection({
  host: "localhost",
  user: "root",
  password: "",   // ganti sesuai konfigurasi kamu
  database: "sensor_db",
});

db.connect((err) => {
  if (err) throw err;
  console.log("✅ Connected to MySQL Database");
});

module.exports = db;
