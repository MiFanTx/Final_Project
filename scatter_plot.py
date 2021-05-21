import os
import numpy as np
import pandas as pd
from cv2 import cv2 as cv

from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestRegressor
from sklearn.metrics import accuracy_score

data_path_left = "Data\\left_final_transposed"
data_path_right = "Data\\right_final_transposed"

def read_data(data_path, frame_index, label):
	csv_final = pd.DataFrame([])
	for file in os.listdir(data_path):
		if file.startswith("frame") and file.endswith(".csv"):
			csv_path = os.path.join(data_path, file)
			csv_raw = pd.read_csv(csv_path, sep = ';', header = None)
			csv_single_frame = csv_raw.iloc[[frame_index]].copy()
			# csv_single_frame["label"] = label
			if csv_final.empty:
				csv_final = csv_single_frame.copy()
			else:
				csv_final = pd.concat([csv_final, csv_single_frame], axis = 0)
	
	csv_final = csv_final.drop(0, axis = 1)

	return csv_final



for frame_index in range (1):

	data_left = read_data(data_path_left, frame_index, [0])
	data_right = read_data(data_path_right, frame_index, [1])

	data_left = np.array(data_left)
	data_right = np.array(data_right)

	data_left = data_left[:, 58:60]
	data_right = data_right[:, 58:60]

	blank_image = np.zeros((720, 720, 3), np.uint8)

	for i in range(50):
		# cv.drawMarker(blank_image, (int(data_left[i, 0]), int(data_left[i, 1])), (0, 0, 255), markerType = 0, markerSize= 1)
		# cv.drawMarker(blank_image, (int(data_right[i, 0]), int(data_right[i, 1])), (255, 0, 0), markerType = 0, markerSize= 5)

		if (blank_image[int(data_left[i, 1]), int(data_left[i, 0]) - 280] == (0,0,0)).all():
			blank_image[int(data_left[i, 1]), int(data_left[i, 0]) - 280] = (0, 0, 255)

		if (blank_image[int(data_left[i, 1]), int(data_left[i, 0]) - 280] == (255,0,0)).all():
			blank_image[int(data_left[i, 1]), int(data_left[i, 0]) - 280] = (255, 255, 255)

		if (blank_image[int(data_right[i, 1]), int(data_right[i, 0]) - 280] == (0,0,0)).all():
			blank_image[int(data_right[i, 1]), int(data_right[i, 0]) - 280] = (255, 0, 0)

		if (blank_image[int(data_right[i, 1]), int(data_right[i, 0]) - 280] == (0,0,255)).all():
			blank_image[int(data_right[i, 1]), int(data_right[i, 0]) - 280] = (255, 255, 255)

	cv.imshow("left_knee", blank_image)
	cv.waitKey(0)