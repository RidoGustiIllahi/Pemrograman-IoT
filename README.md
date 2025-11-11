Buat project Node.Js baru dengan perintah
npm init -y

Install Depedensi
npm install express mysql2 mqtt body-parser

Struktur file
backend/
├── server.js
├── db.js
└── public
    └── index.html

Buat project ESP32 di Wokwi, gunakan program ino.

siapkan MQTT

jalankan backend
node server.js
