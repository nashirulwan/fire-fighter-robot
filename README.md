# Fire Fighter Robot ML

Project ini menambahkan machine learning ke robot pemadam api Arduino berbasis flame sensor. Kode awal robot sudah dapat bergerak berdasarkan api, tetapi keputusan masih memakai threshold manual dan servo nozzle masih bergerak sweeping. Versi ini menambahkan model **Decision Tree Classifier** agar robot dapat menentukan arah api dari data sensor, sehingga servo nozzle mengikuti arah api.

## Konsep Sistem

Input model berasal dari flame sensor 5 channel pada Arduino:

| Arduino | Dataset |
|---|---|
| A0 | `s1` |
| A1 | `s2` |
| A2 | `s3` |
| A3 | `s4` |
| A4 | `s5` |

Output klasifikasi (4 kelas):

| Label | Aksi Robot |
|---|---|
| `NO_FIRE` | Robot berhenti, pompa mati, servo tengah |
| `FIRE_LEFT` | Robot belok kiri, servo nozzle kiri |
| `FIRE_CENTER` | Robot maju, servo nozzle tengah |
| `FIRE_RIGHT` | Robot belok kanan, servo nozzle kanan |

Deteksi **proximity** (api sangat dekat) dipertahankan sebagai threshold deterministik pada channel tengah (A2/s3), terpisah dari model ML. Jika hasilnya `FIRE_CENTER` dan nilai s3 ≤ `PROXIMITY_THRESHOLD`, robot berhenti dan menyiram.

## Metodologi

Notebook training disusun mengikuti alur materi data science:

1. Business Understanding
2. Analytic Approach
3. Data Requirement dan Data Collection
4. Data Understanding
5. Data Preparation
6. Feature Engineering
7. Modeling Scenario
8. Evaluation
9. Model Interpretation
10. Deployment ke Arduino

Algoritma: **Decision Tree Classifier** — ringan, mudah dijelaskan, dan hasilnya dapat diekspor menjadi kode `if-else` untuk Arduino tanpa library ML tambahan.

## Struktur Folder

```text
arduino/
  01_sensor_logger/
    01_sensor_logger.ino      # Baca 5 flame sensor A0-A4 untuk ambil dataset asli
  02_fire_robot_ml/
    02_fire_robot_ml.ino      # Kode final robot dengan Decision Tree

data/
  dummy_dataset_fire_robot.csv  # Dataset dummy (5 kolom, 4 kelas) untuk validasi pipeline
  fire_dataset_real.csv         # Dataset asli dari robot (isi sendiri setelah pengambilan data)

ml/
  train_fire_model.ipynb        # Notebook metodologi data science
  train_fire_model.py           # Script training Decision Tree
```

## Cara Menjalankan Notebook

Jika memakai Nix:

```bash
cd fire-fighting-robot-ml
nix --extra-experimental-features nix-command --extra-experimental-features flakes shell --impure --expr 'with import <nixpkgs> {}; python3.withPackages (ps: with ps; [ scikit-learn pandas matplotlib notebook ])' --command jupyter notebook
```

Buka file:

```text
ml/train_fire_model.ipynb
```

Jika memakai Google Colab, upload:

```text
ml/train_fire_model.ipynb
data/dummy_dataset_fire_robot.csv   ← atau fire_dataset_real.csv setelah data asli tersedia
```

## Training Model

Untuk menjalankan training lewat terminal (Nix):

```bash
nix --extra-experimental-features "nix-command flakes" shell --impure \
  --expr 'with import <nixpkgs> {}; python3.withPackages (ps: with ps; [ scikit-learn pandas ])' \
  --command python ml/train_fire_model.py data/dummy_dataset_fire_robot.csv
```

Output training menghasilkan fungsi Arduino yang langsung bisa ditempel:

```cpp
int predictFireDirection(int s1, int s2, int s3, int s4, int s5) {
  // aturan Decision Tree hasil training
}
```

## Alur Pengambilan Data Asli

1. Upload `arduino/01_sensor_logger/01_sensor_logger.ino` ke robot.
2. Buka Serial Monitor 9600 baud — akan muncul header `s1,s2,s3,s4,s5`.
3. Ambil data per sesi posisi api:
   - Tidak ada api → copy ke kolom `NO_FIRE`
   - Api di depan kiri (A0/A1) → copy ke kolom `FIRE_LEFT`
   - Api di depan tengah (A2) → copy ke kolom `FIRE_CENTER`
   - Api di depan kanan (A3/A4) → copy ke kolom `FIRE_RIGHT`
4. Tambahkan kolom `label` manual, simpan sebagai `data/fire_dataset_real.csv`:

```csv
s1,s2,s3,s4,s5,label
930,910,900,920,940,NO_FIRE
320,460,850,890,900,FIRE_LEFT
750,720,320,730,760,FIRE_CENTER
900,890,850,460,320,FIRE_RIGHT
```

5. Training ulang dengan data asli:

```bash
python ml/train_fire_model.py data/fire_dataset_real.csv
```

6. Salin fungsi `predictFireDirection()` hasil training ke `arduino/02_fire_robot_ml/02_fire_robot_ml.ino`.
7. Sesuaikan nilai `PROXIMITY_THRESHOLD` berdasarkan pembacaan s3 saat api sangat dekat.
8. Upload ke robot dan uji.

## Status Progres

Sudah selesai:
- Dataset dummy 4 kelas, 5 kolom untuk validasi pipeline.
- Notebook training dengan metodologi data science (80:20 split).
- Script training Decision Tree.
- Arduino sensor logger — baca semua 5 channel.
- Arduino final dengan servo mengikuti arah klasifikasi + proximity check deterministik.

Belum selesai:
- Pengambilan dataset asli dari robot fisik.
- Training ulang dengan data asli.
- Kalibrasi `PROXIMITY_THRESHOLD` dari data nyata.
- Upload dan uji langsung pada robot.
