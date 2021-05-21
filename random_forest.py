import os
import numpy as np
import pandas as pd
import matplotlib.pyplot as plt
from cv2 import cv2 as cv

from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestRegressor
from sklearn.metrics import accuracy_score



data_path_left = "Data\\left_final\\"
data_path_right = "Data\\right_final\\"

epoch = 5

def read_data(data_path, frame_index, label):
	csv_final = pd.DataFrame([])
	for file in os.listdir(data_path):
		if file.startswith("frame") and file.endswith(".csv"):
			csv_path = os.path.join(data_path, file)
			csv_raw = pd.read_csv(csv_path, sep = ';', header = None)
			csv_single_frame = csv_raw.iloc[[frame_index]].copy()
			if csv_final.empty:
				csv_final = csv_single_frame.copy()
			else:
				csv_final = pd.concat([csv_final, csv_single_frame], axis = 0)
	
	csv_final = csv_final.drop(0, axis = 1)

	return csv_final

def random_forest(data_1, data_2, tree_number, random_number):

	data_size = data_1.shape[0]
	label_1 = np.zeros(data_size)
	label_2 = np.ones(data_size)

	train_features_1, test_features_1, train_labels_1, test_labels_1 = train_test_split(data_1, label_1, test_size = 0.2, random_state = random_number)
	train_features_2, test_features_2, train_labels_2, test_labels_2 = train_test_split(data_2, label_2, test_size = 0.2, random_state = random_number)

	train_features = np.vstack((train_features_1, train_features_2))
	test_features = np.vstack((test_features_1, test_features_2))
	train_labels = np.hstack((train_labels_1, train_labels_2))
	test_labels = np.hstack((test_labels_1, test_labels_2))

	rf = RandomForestRegressor(n_estimators = tree_number, random_state = random_number)
	rf.fit(train_features, train_labels)
	predictions = rf.predict(test_features)

	predictions[predictions >= 0.5] = 1
	predictions[predictions < 0.5] = 0
	accuracy = accuracy_score(test_labels, predictions)
	return accuracy




for frame_index in range (10):

	data_left = read_data(data_path_left, frame_index, [0])
	data_right = read_data(data_path_right, frame_index, [1])

	data_left = np.array(data_left)
	data_right = np.array(data_right)

	# data_left = data_left[:, 69:72]
	# data_right = data_right[:, 69:72]

	score = 0

	for i in range (epoch):
		score += random_forest(data_left, data_right, 1000, 10)
	average = score / epoch
	average = round(average, 2)

	print(str(frame_index) + ": " + str(average))

