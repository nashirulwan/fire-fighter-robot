# Fire Fighter Robot ML

Project ini menambahkan machine learning ke robot pemadam api Arduino berbasis flame sensor. Kode awal robot sudah dapat bergerak berdasarkan api, tetapi keputusan masih memakai threshold manual dan servo nozzle masih bergerak sweeping. Versi ini menambahkan model **Decision Tree Classifier** agar robot dapat menentukan kondisi api dari data sensor.

## Konsep Sistem

Input model berasal dari flame sensor 5 channel pada Arduino:

| Arduino | Dataset |
|---|---|
| A0 | `s1` |
| A1 | `s2` |
| A2 | `s3` |
| A3 | `s4` |
| A4 | `s5` |

Output klasifikasi:

| Label | Aksi Robot |
|---|---|
| `NO_FIRE` | Robot berhenti, pompa mati, servo tengah |
| `FIRE_LEFT` | Robot belok kiri, servo nozzle kiri |
| `FIRE_CENTER` | Robot maju, servo nozzle tengah |
| `FIRE_RIGHT` | Robot belok kanan, servo nozzle kanan |
| `FIRE_CLOSE` | Robot berhenti, pompa menyala, servo menyiram arah sensor terkuat |

## Metodologi Data Science

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

Metode yang digunakan hanya **Decision Tree Classifier** karena ringan, mudah dijelaskan, dan hasilnya dapat diekspor menjadi kode `if-else` untuk Arduino.

## Struktur Folder

```text
arduino/
  01_sensor_logger/
    01_sensor_logger.ino      # Membaca flame sensor A0-A4 untuk ambil data asli
  02_fire_robot_ml/
    02_fire_robot_ml.ino      # Draft kode final robot dengan Decision Tree

data/
  dummy_dataset_fire_robot.csv # Dataset dummy untuk progres awal

ml/
  train_fire_model.ipynb       # Notebook metodologi data science
  train_fire_model.py          # Script training Decision Tree
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
data/dummy_dataset_fire_robot.csv
```

Lalu sesuaikan `dataset_path` bila diperlukan.

## Training Model

Untuk menjalankan training lewat terminal:

```bash
python ml/train_fire_model.py data/dummy_dataset_fire_robot.csv
```

Hasil sementara dari dummy dataset:

```text
Decision Tree Accuracy: 0.920
```

Output training menghasilkan fungsi Arduino:

```cpp
int predictFireDirection(int s1, int s2, int s3, int s4, int s5) {
  // aturan Decision Tree hasil training
}
```

Fungsi tersebut ditempelkan ke `arduino/02_fire_robot_ml/02_fire_robot_ml.ino`.

## Rencana Pengujian Lab

1. Upload `arduino/01_sensor_logger/01_sensor_logger.ino` ke robot.
2. Buka Serial Monitor 9600 baud.
3. Ambil data untuk `NO_FIRE`, `FIRE_LEFT`, `FIRE_CENTER`, `FIRE_RIGHT`, dan `FIRE_CLOSE`.
4. Simpan sebagai `data/fire_dataset_real.csv` dengan format:

```csv
s1,s2,s3,s4,s5,label
930,910,900,920,940,NO_FIRE
900,320,720,810,850,FIRE_LEFT
860,780,320,790,850,FIRE_CENTER
850,810,720,330,900,FIRE_RIGHT
840,780,860,790,830,FIRE_CLOSE
```

5. Training ulang memakai data asli.
6. Salin fungsi `predictFireDirection()` hasil training asli ke kode Arduino final.
7. Upload `arduino/02_fire_robot_ml/02_fire_robot_ml.ino` ke robot.
8. Uji motor, servo nozzle, dan pompa berdasarkan hasil klasifikasi.

## Status Progres

Sudah selesai:

- Dataset dummy untuk validasi pipeline.
- Notebook training dengan metodologi data science.
- Script training Decision Tree.
- Arduino data logger.
- Draft Arduino final dengan servo mengikuti hasil klasifikasi.

Belum selesai:

- Pengambilan dataset asli dari robot.
- Training ulang dengan data asli.
- Upload dan uji langsung pada robot fisik.
