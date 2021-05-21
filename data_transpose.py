import os 
import numpy as np
import pandas as pd
from cv2 import cv2 as cv

import random


# Read image
data_path = "Data\\right_final"
for file in os.listdir(data_path):
	if file.startswith("frame") and file.endswith(".csv"):
		random_number_x = random.uniform(-50, 50)
		random_number_y = random.uniform(-50, 50)
		serial_number = file[:-4]
		csv_path = os.path.join(data_path, file)
		csv_raw = pd.read_csv(csv_path, sep = ';', header = None)

		for i in range(103):
			if (i - 2) % 3 == 0 or i >= 97:
				csv_raw.iloc[:, i] += random_number_x

		for i in range(103):
			if (i - 1) % 3 == 0 or i >= 97:	
				csv_raw.iloc[:, i] += random_number_x

		csv_raw.to_csv(data_path + "_transposed\\" + serial_number + ".csv", index = False, header = False, sep = ";")