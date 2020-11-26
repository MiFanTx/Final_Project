#include <k4a/k4a.hpp>
#include <k4abt.hpp>
#include <k4arecord/record.h>
#include <k4arecord/playback.h>

#include <iostream>
#include <fstream>
#include <chrono>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;



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
	device_config.camera_fps = K4A_FRAMES_PER_SECOND_15;
	device_config.depth_mode = K4A_DEPTH_MODE_NFOV_UNBINNED;
	device_config.color_format = K4A_IMAGE_FORMAT_COLOR_BGRA32;
	device_config.color_resolution = K4A_COLOR_RESOLUTION_1080P;
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

	k4a::image depthImage;
	k4a::image colorImage;
	k4a::image irImage;

	cv::Mat depthFrame;
	cv::Mat colorFrame;
	cv::Mat irFrame;

	k4abt::frame body_frame = NULL;

	// Calibration and body tracker
	k4a_calibration_t calibration = device.get_calibration(device_config.depth_mode, device_config.color_resolution);
	k4abt::tracker tracker = NULL;
	k4abt_tracker_configuration_t tracker_config = K4ABT_TRACKER_CONFIG_DEFAULT;
	tracker = k4abt::tracker::create(calibration, tracker_config);

	// Creat output file
	string result = "";
	ofstream outfile("result.txt");

	// Time measurement
	typedef chrono::high_resolution_clock clock;
	typedef chrono::duration<float, milli> mili;

	

	// Main loop to process data
	while (1)
	{
		// Get capture immediately
		if (device.get_capture(&capture, chrono::milliseconds(0)))
		{
			//auto t0 = clock::now();

			// Sensor code
			
			//k4a::image depthImage = capture.get_depth_image();
			//k4a::image irImage = capture.get_ir_image();
			k4a::image colorImage = capture.get_color_image();

			uint8_t* colorTextureBuffer = colorImage.get_buffer();
			cv::Mat colorFrame = cv::Mat(colorImage.get_height_pixels(),
										 colorImage.get_width_pixels(),
										 CV_8UC4, colorTextureBuffer);

			cv::imshow("Kinect color frame", colorFrame);

			// Body Tracking code

			tracker.enqueue_capture(capture);
			tracker.pop_result(&body_frame);

			string frame_result = "";

			if (body_frame.get_num_bodies() > 0) {

				k4abt_body_t body = body_frame.get_body(0);

				for (int i = 0; i < 32; i++) {
					frame_result += to_string(body.skeleton.joints[i].confidence_level);
					frame_result += ",";
					frame_result += to_string(body.skeleton.joints[i].position.xyz.x);
					frame_result += ",";
					frame_result += to_string(body.skeleton.joints[i].position.xyz.y);
					frame_result += ",";
					frame_result += to_string(body.skeleton.joints[i].position.xyz.z);
					frame_result += ",";
					frame_result += to_string(body.skeleton.joints[i].orientation.wxyz.x);
					frame_result += ",";
					frame_result += to_string(body.skeleton.joints[i].orientation.wxyz.y);
					frame_result += ",";
					frame_result += to_string(body.skeleton.joints[i].orientation.wxyz.z);
					frame_result += ",";
					frame_result += to_string(body.skeleton.joints[i].orientation.wxyz.w);
					frame_result += ",";
				}
				// Remove last comma
				frame_result.pop_back();
				result += frame_result;
				result += "\n";
				
			}

			//auto t1 = clock::now();
			//cout << mili(t1 - t0).count() << "ms\n";

			// Manual quit option CV
			if (waitKey(30) == 27){
				cout << "User Interepted" << endl;
				break;
			}
		}
	}

	// Write to output file
	outfile << result << endl;

	// Stops camera and close device before exit
	device.stop_cameras();
	device.close();
}