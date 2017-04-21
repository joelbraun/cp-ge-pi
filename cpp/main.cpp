#include <stdio.h>
#include <string>
#include <sstream>
#include <fstream>
#include <climits>
#include <ctime>
#include <opencv2/opencv.hpp>
#include "Parking.h"
#include "utils.h"
#include "ConfigLoad.h"
#include "restclient-cpp/connection.h"
#include "restclient-cpp/restclient.h"

using namespace std;

RestClient::Connection* getRestClient() {
	RestClient::init();
	
	RestClient::Connection* conn = new RestClient::Connection("https://cp-parking-process-api.run.aws-usw02-pr.ice.predix.io");
	RestClient::HeaderFields headers;
	headers["Content-Type"] = "application/json";
	conn->SetHeaders(headers);

	return conn;
}

void postRequest(RestClient::Connection* conn, map<int, bool> data) 
{
	string postData = "{";
		postData += "\"ParkingLotId\": \"ParkingLot1\",";
		postData += "\"Timestamp\": \"";
		char time_buf[21];
		time_t now;
		time(&now);
	    strftime(time_buf, 21, "%Y-%m-%dT%H:%S:%MZ", localtime(&now));
		postData += time_buf;
		postData += "\",";
		postData += "\"ParkingSpots\":["; 
	for (int i = 0; i < data.size(); i++)
	{
		postData += "{";
		postData += "\"id\":";
		postData += to_string(i);
		postData += ", \"status\": ";
		postData += data[i] ? "true" : "false";
		postData += "}";
		if (i + 1 != data.size())
			postData += ",";
	}
	postData += "]}";

	cout << postData << endl;

	RestClient::Response r = conn->post("/api/processing", postData);
	cout << r.code << endl;
}

int main(int argc, char** argv)
{
	if (argc != 3)
	{
		printf("usage: DetectParking.exe <Video_Filename or Camera_Number> <ParkingData_Filename>\n\n");
		printf("<Camera_Number> can be 0, 1, ... for attached cameras\n");
		printf("<ParkingData_Filename> should be simple txt file with lines of: id x1 y1 x2 y2 x3 y3 x4 y4\n");
		return -1;
	}
    
    //Load configs
    ConfigLoad::parse();
    
	const string videoFilename = argv[1];	
	vector<Parking>  parking_data = parse_parking_file(argv[2]);
	
	// Open Camera or Video	File
	cv::VideoCapture cap;
	if (videoFilename == "0" || videoFilename == "1" || videoFilename == "2")
	{
		printf("Loading Connected Camera...\n");
		cap.open(stoi(videoFilename));
		cv::waitKey(500);
	}
	else 
	{
		cap.open(videoFilename);
	}	
	if (!cap.isOpened())
	{
		cout << "Could not open: " << videoFilename << endl;
		return -1;
	}
	cv::Size videoSize = cv::Size((int)cap.get(cv::CAP_PROP_FRAME_WIDTH),    // Acquire input size
		(int)cap.get(cv::CAP_PROP_FRAME_HEIGHT));

	cv::VideoWriter outputVideo;
    if (ConfigLoad::options["SAVE_VIDEO"] == "true")
	{		
		string::size_type pAt = videoFilename.find_last_of('.');                  // Find extension point
		const string videoOutFilename = videoFilename.substr(0, pAt) + "_out.avi";   // Form the new name with container		
		//cv::VideoWriter::CV_FOURCC('C', 'R', 'A', 'M');
		outputVideo.open(videoOutFilename, -1, cap.get(cv::CAP_PROP_FPS), videoSize, true);
	}

	// Initiliaze variables
	cv::Mat frame, frame_blur, frame_gray, frame_out, roi, laplacian;
	cv::Scalar delta, color;
	ostringstream oss;
	cv::Size blur_kernel = cv::Size(5, 5); 	

	unsigned int frameCt = 0;
	RestClient::Connection* conn = getRestClient();
	// Loop through Video
	while (cap.isOpened())
	{
		cap.read(frame);
		if (frame.empty())
		{
			printf("Error reading frame\n");
			return -1;
		}
		
		frame_out = frame.clone();
		cv::cvtColor(frame, frame_gray, cv::COLOR_BGR2GRAY);
		cv::GaussianBlur(frame_gray, frame_blur, blur_kernel, 3, 3);
		
		if (ConfigLoad::options["DETECT_PARKING"] == "true")
		{
			map<int, bool> pStatus;
			for (Parking& park : parking_data)
			{
				// Check if parking is occupied
				roi = frame_blur(park.getBoundingRect());
				cv::Laplacian(roi, laplacian, CV_64F);								
				delta = cv::mean(cv::abs(laplacian), park.getMask());
                park.setStatus( delta[0] < atof(ConfigLoad::options["PARK_LAPLACIAN_TH"].c_str()) );
				//printf("| %d: d=%-5.1f", park.getId(), delta[0]);
				pStatus[park.getId()] = delta[0] > atof(ConfigLoad::options["PARK_LAPLACIAN_TH"].c_str());

			}
			if (frameCt % atoi(ConfigLoad::options["SAMPLE_RATE"].c_str()) == 0) {
				postRequest(conn, pStatus);
			}
			frameCt++;
			if (frameCt == UINT_MAX){
				frameCt = 0;
			}
		}

		// Save Video
		if (ConfigLoad::options["SAVE_VIDEO"] == "true")
		{
			outputVideo.write(frame_out);
		}
	}
	RestClient::disable();
	return 0;
}

