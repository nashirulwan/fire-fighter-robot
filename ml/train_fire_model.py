#!/usr/bin/env python3
import csv
import sys
from pathlib import Path

try:
    from sklearn.metrics import accuracy_score, classification_report, confusion_matrix
    from sklearn.model_selection import train_test_split
    from sklearn.tree import DecisionTreeClassifier, export_text
except ImportError as exc:
    raise SystemExit(
        "scikit-learn belum terpasang. Install dengan: python3 -m pip install scikit-learn"
    ) from exc


FEATURES = ["s1", "s2", "s3", "s4", "s5"]
LABEL_ORDER = ["NO_FIRE", "FIRE_LEFT", "FIRE_CENTER", "FIRE_RIGHT"]


def load_dataset(path: Path):
    rows = []
    labels = []
    with path.open(newline="") as file:
        reader = csv.DictReader(file)
        for row in reader:
            rows.append([int(row[f]) for f in FEATURES])
            labels.append(row["label"].strip())
    return rows, labels


def arduino_tree(node, tree, feature_names, class_names, indent="  "):
    left = tree.children_left[node]
    right = tree.children_right[node]

    if left == right:
        class_index = tree.value[node][0].argmax()
        return f"{indent}return {class_names[class_index]};\n"

    feature = feature_names[tree.feature[node]]
    threshold = int(round(tree.threshold[node]))

    code = f"{indent}if ({feature} <= {threshold}) {{\n"
    code += arduino_tree(left, tree, feature_names, class_names, indent + "  ")
    code += f"{indent}}} else {{\n"
    code += arduino_tree(right, tree, feature_names, class_names, indent + "  ")
    code += f"{indent}}}\n"
    return code


def main():
    dataset_path = Path(sys.argv[1]) if len(sys.argv) > 1 else Path("data/dummy_dataset_fire_robot.csv")
    x, y = load_dataset(dataset_path)

    unknown = set(y) - set(LABEL_ORDER)
    if unknown:
        raise SystemExit(f"Label tidak dikenal dalam dataset: {unknown}")
    if len(set(y)) < 2:
        raise SystemExit("Dataset harus punya minimal 2 label berbeda.")

    x_train, x_test, y_train, y_test = train_test_split(
        x, y, test_size=0.20, random_state=42, stratify=y
    )

    tree_model = DecisionTreeClassifier(max_depth=4, random_state=42)
    tree_model.fit(x_train, y_train)
    prediction = tree_model.predict(x_test)

    print("\nDecision Tree")
    print(f"Accuracy: {accuracy_score(y_test, prediction):.3f}")
    print("\nDecision Tree rules:")
    print(export_text(tree_model, feature_names=FEATURES))
    print("Confusion matrix labels:", tree_model.classes_.tolist())
    print(confusion_matrix(y_test, prediction, labels=tree_model.classes_))
    print("\nClassification report:")
    print(classification_report(y_test, prediction))

    class_names = list(tree_model.classes_)

    print("\nArduino enum:")
    print("enum FireDirection {")
    for i, label in enumerate(LABEL_ORDER):
        print(f"  {label} = {i},")
    print("};")

    print("\nArduino predictFireDirection():")
    print("int predictFireDirection(int s1, int s2, int s3, int s4, int s5) {")
    print(arduino_tree(0, tree_model.tree_, FEATURES, class_names), end="")
    print("}")


if __name__ == "__main__":
    main()
