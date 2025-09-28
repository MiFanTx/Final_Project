# Badminton Serve Prediction Using Azure Kinect & Random Forest

A system to record badminton serves, track the player skeleton using **Azure Kinect**, extract features, and predict serve type using a **Random Forest** model.

---

## Project Overview

This project aims to:

1. Record badminton serves captured via an **Azure Kinect** camera.  
2. Track full-body skeleton landmarks during each serve.  
3. Extract features (joint angles, racket positions).  
4. Train a **Random Forest classifier** to recognise the direction of the serve (e.g. left or right) from the motion data.  
5. Visualise predictions and data relationships.

By combining depth/skeleton tracking and machine learning, this project provides insights into serve biomechanics and enables automatic classification of serve direction.

---

## Repository Structure

```text
.
├── Kinect_Application.cpp         # C++ program to capture depth & skeleton via Azure Kinect SDK
├── Kinect_Application.sln         # Visual Studio solution file
├── data_processing.py             # Process raw skeleton data, feature extraction
├── data_transpose.py              # Utility to reshape/reorganise data
├── random_forest.py               # Train & evaluate Random Forest on features
├── scatter_plot.py                # Visualisation tools: plots, feature importance, etc.
├── packages.config                # Dependencies for the C++ Kinect project
└── README.md
