#include <k4a/k4a.hpp>
#include <k4abt.hpp>
#include <k4arecord/record.h>
#include <k4arecord/playback.h>

#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

const int max_frames = 10;
const int max_iteration = 30;
const int frame_width = 1280;
const int frame_height = 720;
// Start recording 0.5s after the starting sign
const int time_gap = 5500000;
const string path_left = "Data\\left_new\\frame_";
const string path_right = "Data\\right_new\\frame_";


int main(int argc, char*& argv)
{

	// Check connected device
	const auto device_count = k4a::device::get_installed_count();
	if (!device_count)
	{
		cout << "No Azure Kinect devices detected!" << endl;
		return EXIT_FAILURE;
	}
	else
	{
		cout << "Current connected device number: " << device_count << endl;
	}

	// Configure camera options
	k4a_device_configuration_t device_config = K4A_DEVICE_CONFIG_INIT_DISABLE_ALL;
	device_config.camera_fps = K4A_FRAMES_PER_SECOND_30;
	device_config.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
	device_config.color_format = K4A_IMAGE_FORMAT_COLOR_BGRA32;
	device_config.color_resolution = K4A_COLOR_RESOLUTION_720P;
	device_config.synchronized_images_only = true;

	
	// Open the first plugged in kinect device, start the camera with given configuration
	k4a::device device = NULL;
	string device_serial_number;
	cout << "Opening the first connected device ..." << endl;
	try
	{
		device = k4a::device::open(K4A_DEVICE_DEFAULT);
		device_serial_number = device.get_serialnum();
		device.start_cameras(&device_config);
	}
	catch (k4a::error error)
	{
		cout << error.what() << endl;
	}
	cout << "Device: " << device_serial_number << " is opened successfully." << endl;


	// Initialise variables
	k4a::capture capture;

	k4a::image depth_image;
	k4a::image color_image;
	k4a::image ir_image;

	cv::Mat depth_frame;
	cv::Mat color_frame;
	cv::Mat ir_frame;

	k4abt::frame body_frame = NULL;

	// Calibration and body tracker
	k4a::calibration calibration = device.get_calibration(device_config.depth_mode, device_config.color_resolution);
	k4a::transformation transformation;
	k4abt::tracker tracker = NULL;
	k4abt_tracker_configuration_t tracker_config = K4ABT_TRACKER_CONFIG_DEFAULT;
	tracker = k4abt::tracker::create(calibration, tracker_config);

	// Time measurement
	typedef chrono::high_resolution_clock clock;
	typedef chrono::duration<float, milli> milli;
	chrono::nanoseconds initial_time_stamp;
	

	// pure color mat
	cv::Mat red_frame(frame_height, frame_width, CV_8UC3, Scalar(0, 0, 255));
	cv::Mat green_frame(frame_height, frame_width, CV_8UC3, Scalar(0, 255, 0));
	cv::Mat yellow_frame(frame_height, frame_width, CV_8UC3, Scalar(0, 255, 255));


	// Exposure
	device.set_color_control(K4A_COLOR_CONTROL_EXPOSURE_TIME_ABSOLUTE, K4A_COLOR_CONTROL_MODE_MANUAL, 10000);

	// Main loop to process data
	for (int iteration = 0; iteration < max_iteration; iteration++)
	{
		initial_time_stamp = chrono::nanoseconds{ -1 };

		// Set file name with current date and time
		auto now = time(0);
		auto local_time = localtime(&now);
		string time_string = to_string(1900 + local_time->tm_year) + "_"
			+ to_string(1 + local_time->tm_mon) + "_"
			+ to_string(local_time->tm_mday) + "_"
			+ to_string(local_time->tm_hour) + "_"
			+ to_string(local_time->tm_min) + "_"
			+ to_string(local_time->tm_sec);

		// Create output csv
		string data_file_name = path_right + time_string + ".csv";
		ofstream outfile(data_file_name);
		string result = "";

		// Create output image
		string image_file_name = path_right + time_string;
		cv::Mat image_array[max_frames];

		// count how many frames have been recorded in one iteration
		int frame_index = 0;

		while (1)
		{
			// Get capture immediately
			if (device.get_capture(&capture, chrono::milliseconds(0)))
			{
				auto frame_start_time = clock::now();

				// Get color image

				k4a::image color_image = capture.get_color_image();

				uint8_t* colorTextureBuffer = color_image.get_buffer();
				cv::Mat colorFrame = cv::Mat(color_image.get_height_pixels(), color_image.get_width_pixels(), CV_8UC4, colorTextureBuffer);

				// Initialise time stamp with the frame time stamp
				if (initial_time_stamp.count() < 0) {
					initial_time_stamp = color_image.get_system_timestamp();
				}

				// Relative time past the initial time stamp in microseconds
				auto current_time_stamp = color_image.get_system_timestamp();
				auto relative_time = chrono::duration_cast<chrono::microseconds>(current_time_stamp - initial_time_stamp).count();

				// Get body tracking data

				tracker.enqueue_capture(capture);
				tracker.pop_result(&body_frame);

				// Record data only if the time has past 5 seconds
				if (relative_time > time_gap) {

					// If the recorded frame number is less than the max frames number, store image and data
					if (frame_index < max_frames) {

						image_array[frame_index] = colorFrame.clone();

						// String that store the realtive time and joint results. 
						string frame_result = "";
						frame_result += to_string(relative_time - time_gap);
						frame_result += ";";

						if (body_frame.get_num_bodies() > 0) {

							// Get the first recognised body
							k4abt_body_t body = body_frame.get_body(0);
							k4a_float2_t position_2d;

							for (int i = 0; i < 32; i++) {
								// Current Joint
								k4abt_joint_t joint = body.skeleton.joints[i];

								// Convert joint 3d depth coordinate to 2d color coordinate
								calibration.convert_3d_to_2d(joint.position, K4A_CALIBRATION_TYPE_DEPTH, K4A_CALIBRATION_TYPE_COLOR, &position_2d);

								// Draw marker on each joint
								// cv::drawMarker(colorFrame, Point(position_2d.xy.x, position_2d.xy.y), Scalar(0, 0, 255), 0, 20, 2, 8);

								// Write data to output string
								frame_result += to_string(joint.confidence_level);
								frame_result += ";";
								frame_result += to_string(position_2d.xy.x);
								frame_result += ";";
								frame_result += to_string(position_2d.xy.y);
								frame_result += ";";
								/*
								frame_result += to_string(joint.orientation.wxyz.x);
								frame_result += ";";
								frame_result += to_string(joint.orientation.wxyz.y);
								frame_result += ";";
								frame_result += to_string(joint.orientation.wxyz.z);
								frame_result += ";";
								frame_result += to_string(joint.orientation.wxyz.w);
								frame_result += ";";
								*/
							}

							// Remove last comma
							frame_result.pop_back();
							result += frame_result;
							result += "\n";
						}

						frame_index++;
					}

					// If enough frames are recorded, end loop and goto next iteration
					else {
						frame_index = 0;
						break;
					}
				}

				// exposure time
				cout << "exposure: " << color_image.get_exposure().count() << "    ";

				// Show count down with color

				if (relative_time < 3e6){
					cv::imshow("Kinect color frame", colorFrame);
				}
				if (relative_time > 3e6) {
					cv::imshow("Kinect color frame", red_frame);
				}
				if (relative_time > 4e6) {
					cv::imshow("Kinect color frame", yellow_frame);
				}
				if(relative_time > 5e6) {
					cv::imshow("Kinect color frame", green_frame);
				}

				// Timer for each frame
				auto frame_end_time = clock::now();
				cout << milli(frame_end_time -frame_start_time).count() << "ms\n";

				// Manual quit option CV
				if (waitKey(30) == 27) {
					cout << "User Interepted" << endl;
					break;
				}
			}
		}

		// This iteration ends, write data to file.
		for (int i = 0; i < max_frames; i++) {
			// 720*720 square in the middle
			auto cropped_image = image_array[i](Rect(280, 0, 720, 720));
			cv::imwrite(image_file_name + "_" + to_string(i) + ".png", cropped_image);
		}
		outfile << result << endl;
		outfile.close();
	}

	// Stops camera and close device before exit
	device.stop_cameras();
	device.close();
}