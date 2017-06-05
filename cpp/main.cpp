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
#include "RestActions.h"
#include <raspicam/raspicam_cv.h>

using namespace std;

int main(int argc, char** argv)
{
	if (argc != 2)
	{
		printf("usage: DetectParking <ParkingData_Filename>\n\n");
		printf("<ParkingData_Filename> should be simple txt file with lines of: id x1 y1 x2 y2 x3 y3 x4 y4\n");
		return -1;
	}
    
        //Load configs
        ConfigLoad::parse();
    	
	vector<Parking>  parking_data = parse_parking_file(argv[1]);

	// Initiliaze variables
	cv::Mat frame, frame_blur, frame_gray, frame_out, roi, laplacian;
	cv::Scalar delta, color;
	ostringstream oss;
	cv::Size blur_kernel = cv::Size(5, 5); 	

	unsigned int frameCt = 0;
	RestActions* restActions = new RestActions();
        restActions->getProcessOAuthToken();
        restActions->getConfigOAuthToken();
	RestClient::Connection* conn = restActions->getRestClient();
	raspicam::RaspiCam_Cv Camera;
        Camera.set(CV_CAP_PROP_FORMAT, CV_8UC1);
        if (!Camera.open()) {
           cerr << "Error opening camera. Terminating..." << endl;
           return 1;
        }
	// Loop through Video
	while (true)
	{
                //cout << Camera.grab() << endl;
		Camera.retrieve(frame);
		cv::Point2f src_center(frame.cols / 2.0F, frame.rows / 2.0F);
                cv::Mat rot_mat = cv::getRotationMatrix2D(src_center, 90, 1.0);
                cv::warpAffine(frame, frame, rot_mat, frame.size());
                
                string fname = "output.jpg";
                cv::imwrite(fname, frame);
                
                cv::GaussianBlur(frame, frame_blur, blur_kernel, 3, 3);
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
                                system("python /home/pi/cp-ge-pi/cpp/sendimage.py");
				RestClient::Response response = restActions->postRequest(conn, pStatus);
				cout << "Configuration received: " << restActions->receiveConfiguration(response, &parking_data) << endl;
			}
			frameCt++;
			if (frameCt == UINT_MAX){
				frameCt = 0;
			}
		}

	}
	
	return 0;
}

