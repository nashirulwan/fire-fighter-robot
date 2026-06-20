# Penjelasan Singkat `02_fire_robot_ml.ino`

File `arduino/02_fire_robot_ml/02_fire_robot_ml.ino` adalah program utama robot pemadam api berbasis Arduino. Tugas file ini adalah membaca 5 sensor api, menentukan arah sumber api dengan model Decision Tree, lalu menggerakkan motor DC, servo nozzle, dan pompa sesuai hasil keputusan.

## 1. Tujuan Program

Program ini dibuat agar robot dapat:

- mendeteksi arah api dari 5 sensor flame,
- bergerak ke kiri, kanan, maju, atau berhenti,
- mengarahkan nozzle servo ke posisi api,
- menyalakan pompa saat api sudah dianggap dekat.

## 2. Mapping Sensor

Urutan sensor yang dipakai di program adalah:

- `s1 = A5`
- `s2 = A4`
- `s3 = A3`
- `s4 = A2`
- `s5 = A1`

Urutan ini penting karena harus sama dengan urutan data saat training model machine learning.

## 3. Perangkat yang Dikendalikan

Program mengendalikan:

- 2 motor DC untuk gerak robot,
- 1 motor pompa air,
- 1 servo untuk arah semprotan,
- 5 sensor flame sebagai input.

## 4. Alur Program

### a. `setup()`

Saat robot dinyalakan, program melakukan beberapa langkah awal:

1. Mengaktifkan komunikasi serial untuk debugging.
2. Mengatur posisi awal servo di tengah.
3. Mengatur kecepatan awal motor dan pompa.
4. Memastikan motor berhenti saat startup.
5. Melakukan **kalibrasi ambient**.

Kalibrasi ambient berarti robot membaca kondisi cahaya sekitar saat awal hidup, lalu menyimpan nilai dasar tiap sensor sebagai baseline. Tujuannya agar cahaya ruangan biasa tidak langsung dianggap sebagai api.

### b. `loop()`

Di dalam `loop()`, program berjalan berulang dengan urutan berikut:

1. Membaca sensor dengan **smoothing**.
2. Menjalankan model `predictFireDirection(...)`.
3. Mengecek apakah perubahan sensor cukup besar dibanding baseline ambient.
4. Menstabilkan keputusan dengan **hysteresis**.
5. Mengecek apakah api tengah sudah cukup dekat.
6. Menjalankan aksi motor, servo, dan pompa.

## 5. Penjelasan Tiap Tahap

### a. Smoothing Sensor

Program tidak langsung memakai satu pembacaan `analogRead()`. Nilai sensor dirata-ratakan dari beberapa sampel agar noise berkurang. Ini membuat pembacaan lebih stabil dan tidak terlalu sensitif terhadap perubahan cahaya yang sangat cepat.

### b. Prediksi Arah Api

Fungsi `predictFireDirection(s1, s2, s3, s4, s5)` adalah hasil training Decision Tree yang sudah ditempel ke dalam Arduino.

Outputnya ada 4 kemungkinan:

- `NO_FIRE`
- `FIRE_LEFT`
- `FIRE_CENTER`
- `FIRE_RIGHT`

### c. Ambient Gate

Setelah model memberi prediksi, program membandingkan nilai sensor sekarang dengan baseline ambient. Jika perubahan sensor masih kecil, maka hasil dipaksa menjadi `NO_FIRE`.

Fungsi ini penting untuk mengurangi false trigger akibat lampu ruangan, pantulan cahaya, atau kondisi terang yang bukan api.

### d. Hysteresis Keputusan

Keputusan baru tidak langsung dipakai dalam satu loop. Program menunggu hasil klasifikasi yang sama muncul beberapa kali berturut-turut sebelum keputusan dianggap valid.

Tujuannya:

- mengurangi gerakan robot yang bolak-balik,
- mencegah motor sering berubah arah karena pembacaan sesaat,
- membuat perilaku robot lebih stabil.

### e. Proximity Check

Jika hasil akhir adalah `FIRE_CENTER`, program mengecek apakah nilai sensor tengah `s3` sudah melewati `PROXIMITY_THRESHOLD`.

Jika iya:

- robot berhenti,
- pompa menyala,
- servo melakukan gerakan kecil saat menyemprot.

Jika belum:

- robot tetap maju menuju api.

## 6. Logika Aksi Robot

Setelah keputusan akhir didapat, aksi robot adalah:

- `FIRE_LEFT` -> servo ke kiri, robot belok kiri
- `FIRE_RIGHT` -> servo ke kanan, robot belok kanan
- `FIRE_CENTER` -> servo ke tengah, robot maju atau memadamkan
- `NO_FIRE` -> robot berhenti, pompa mati

## 7. Penghalusan Gerakan Motor

Pada versi ini, motor tidak langsung ganti arah secara kasar. Program menambahkan:

- **ramp down** saat mengurangi kecepatan,
- jeda singkat saat ganti arah,
- **ramp up** saat mulai bergerak lagi.

Tujuan bagian ini adalah supaya gerakan motor DC tidak terlalu "jedag-jedug" dan beban mekanik lebih ringan.

## 8. Inti Ide Program

Secara singkat, ide file ini adalah:

1. sensor membaca kondisi api,
2. data dibersihkan dulu dengan smoothing,
3. cahaya sekitar disaring dengan ambient calibration,
4. arah api diputuskan oleh model Decision Tree,
5. keputusan distabilkan dengan hysteresis,
6. robot bergerak dan memadamkan api sesuai hasil keputusan.

## 9. Poin Penting untuk Menjelaskan ke Dosen

- Program ini tidak hanya memakai threshold manual, tetapi memakai model machine learning untuk klasifikasi arah api.
- Sistem tetap memakai aturan tambahan di luar model, yaitu ambient filtering dan proximity check.
- Program sudah memperhatikan kestabilan pembacaan sensor dan kestabilan gerak robot.
- Dengan struktur ini, sistem lebih realistis untuk dipakai pada robot fisik dibanding hanya membaca sensor mentah lalu langsung menggerakkan motor.
