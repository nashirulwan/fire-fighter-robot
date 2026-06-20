### Fire Fighter Robot ML

Adds machine learning to an Arduino flame sensor fire fighting robot. The original robot could already move toward fire, but it decided with manual thresholds and the nozzle servo just swept back and forth. This version adds a Decision Tree Classifier so the robot reads the fire direction from the sensor data and points the nozzle at it.

#### How it works

Model input is a 5 channel flame sensor on the Arduino:

| Arduino | Dataset |
|---|---|
| A5 | `s1` |
| A4 | `s2` |
| A3 | `s3` |
| A2 | `s4` |
| A1 | `s5` |

Output is a 4 class label:

| Label | Robot action |
|---|---|
| `NO_FIRE` | stop, pump off, servo centered |
| `FIRE_LEFT` | turn left, nozzle left |
| `FIRE_CENTER` | move forward, nozzle centered |
| `FIRE_RIGHT` | turn right, nozzle right |

Proximity (fire very close) stays as a deterministic threshold on the center channel (A3/s3), separate from the model. If the prediction is `FIRE_CENTER` and `s3 <= PROXIMITY_THRESHOLD`, the robot stops and sprays.

#### Methodology

The training notebook follows the data science flow from class:

1. Business Understanding
2. Analytic Approach
3. Data Requirement and Data Collection
4. Data Understanding
5. Data Preparation
6. Feature Engineering
7. Modeling Scenario
8. Evaluation
9. Model Interpretation
10. Deployment to Arduino

Algorithm: Decision Tree Classifier. It's light, easy to explain, and the result exports to plain if-else code for the Arduino with no extra ML library.

#### Project structure

```text
arduino/
  01_sensor_logger/
    01_sensor_logger.ino      # read the 5 flame sensors A5-A1 to collect the dataset
  02_fire_robot_ml/
    02_fire_robot_ml.ino      # final robot code with the Decision Tree
data/
  dummy_dataset_fire_robot.csv  # dummy dataset (5 cols, 4 classes) to validate the pipeline
  fire_dataset_real.csv         # real dataset from the robot (you fill this after collecting)
ml/
  train_fire_model.ipynb        # the data science methodology notebook
  train_fire_model.py           # Decision Tree training script
penjelasan/                     # notes and flow/activity diagrams
```

#### Python setup

```bash
python -m venv .venv
source .venv/bin/activate        # on Windows: .\.venv\Scripts\Activate.ps1
pip install -r requirements.txt
```

Then run the training script:

```bash
python ml/train_fire_model.py data/dummy_dataset_fire_robot.csv
```

Or open the notebook (`ml/train_fire_model.ipynb`). Colab works too, just upload the notebook plus a dataset csv.

#### Training

```bash
python ml/train_fire_model.py data/dummy_dataset_fire_robot.csv
```

Training prints an Arduino function you can paste straight into the robot code:

```cpp
int predictFireDirection(int s1, int s2, int s3, int s4, int s5) {
  // Decision Tree rules from training
}
```

#### Collecting real data

1. Upload `arduino/01_sensor_logger/01_sensor_logger.ino` to the robot.
2. Open Serial Monitor at 9600 baud, you'll see the header `s1,s2,s3,s4,s5`.
3. Record per fire position: no fire to `NO_FIRE`, fire front-left (A5/A4) to `FIRE_LEFT`, front-center (A3) to `FIRE_CENTER`, front-right (A2/A1) to `FIRE_RIGHT`.
4. Add a `label` column and save as `data/fire_dataset_real.csv`:

```csv
s1,s2,s3,s4,s5,label
930,910,900,920,940,NO_FIRE
320,460,850,890,900,FIRE_LEFT
750,720,320,730,760,FIRE_CENTER
900,890,850,460,320,FIRE_RIGHT
```

5. Retrain with the real data: `python ml/train_fire_model.py data/fire_dataset_real.csv`
6. Copy the generated `predictFireDirection()` into `arduino/02_fire_robot_ml/02_fire_robot_ml.ino`.
7. Tune `PROXIMITY_THRESHOLD` from the center sensor (A3/s3) reading when the fire is very close.
8. Upload and test.

#### Status

Done:
- Dummy dataset, 4 classes, 5 columns, to validate the pipeline
- Training notebook with the data science methodology (80:20 split)
- Decision Tree training script
- Arduino sensor logger reading all 5 channels
- Final Arduino code with the servo following the predicted direction plus the deterministic proximity check

Not done yet:
- Collecting the real dataset from the physical robot
- Retraining on real data
- Calibrating `PROXIMITY_THRESHOLD` from real readings
- Uploading and testing on the robot

#### License

MIT, see LICENSE.
