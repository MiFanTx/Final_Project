import os 
import numpy as np
import pandas as pd
from cv2 import cv2 as cv

import re

thresh = 80

def color_segmentation(image):
	# define list of boundaries [R, G, B]
	boundaries = [
				([120,0,0], [255,100,100]),
				([0,50,0], [50,255,50]),
				([0,0,200], [200,200,255])
				]

	result = []

	# loop over each boundary
	for (lower, upper) in boundaries:

		# convert boundary into numpy array
		lower = np.array(lower, dtype = "uint8")
		upper = np.array(upper, dtype = "uint8")

		# find the colors within the boundaries and apply the mask
		mask = cv.inRange(image, lower, upper)
		output = cv.bitwise_and(image, image, mask = mask)

		result.append(center_of_mass(output))

	return result
		

def center_of_mass(image):
	threshold = thresh

	# Convert image to gray and blur it
	src_gray = cv.cvtColor(image, cv.COLOR_BGR2GRAY)
	src_gray = cv.blur(src_gray, (3,3))
	
	# Edge detection
	canny_output = cv.Canny(src_gray, threshold, threshold * 2)
	
	contours, _ = cv.findContours(canny_output, cv.RETR_TREE, cv.CHAIN_APPROX_SIMPLE)
	
	# For every found contour we now apply approximation to polygons with accuracy +-3 and stating that the curve must be closed. 
	# After that we find a bounding rect for every polygon and save it to boundRect. 
	# At last we find a minimum enclosing circle for every polygon and save it to center and radius vectors.
	contours_poly = [None]*len(contours)
	boundRect = [None]*len(contours)
	centers = [None]*len(contours)
	radius = [None]*len(contours)

	for i, c in enumerate(contours):
		i = 0
		contours_poly[i] = cv.approxPolyDP(c, 3, True)
		boundRect[i] = cv.boundingRect(contours_poly[i])
		centers[i], radius[i] = cv.minEnclosingCircle(contours_poly[i])
		return centers[0]
		
	
	# drawing = np.zeros((canny_output.shape[0], canny_output.shape[1], 3), dtype=np.uint8)
	
	# for i in range(len(contours)):
	# 	color = (rng.randint(0,256), rng.randint(0,256), rng.randint(0,256))
	# 	cv.drawContours(drawing, contours_poly, i, color)
	# 	cv.rectangle(drawing, (int(boundRect[i][0]), int(boundRect[i][1])), \
	# 	  (int(boundRect[i][0]+boundRect[i][2]), int(boundRect[i][1]+boundRect[i][3])), color, 2)
	# 	cv.circle(drawing, (int(centers[i][0]), int(centers[i][1])), int(radius[i]), color, 2)
	
	
	# cv.imshow('Contours', drawing)
	# cv.waitKey(0)

	# return


# Read image
data_path = "Data\\right_new"
for file in os.listdir(data_path):
	if file.startswith("frame") and file.endswith(".csv"):
		print(file)
		serial_number = file[:-4]
		csv_path = os.path.join(data_path, file)
		csv_raw = pd.read_csv(csv_path, sep = ';', header = None)
		coordinate_list = []

		for file in os.listdir(data_path):
			if re.match(serial_number + "_[0-9].png", file):
				image = cv.imread(os.path.join(data_path, file))
				result = color_segmentation(image)

				for i in range (3):
					if result[i] == None :
						result[i] = (0,0)

				for (a,b) in result:
					coordinate_list.append(a)
					coordinate_list.append(b)

		coordinate = np.array(coordinate_list)
		coordinate = coordinate.reshape(-1, 6)
		coordinate = pd.DataFrame(coordinate)
		csv_final = pd.concat([csv_raw,coordinate], axis = 1)
		csv_final.to_csv(data_path + "_final\\" + serial_number + ".csv", index = False, header = False, sep = ";")
	