#include "RestActions.h"
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include <unistd.h>
#include <vector>
#include "Parking.h"
#include "restclient-cpp/connection.h"
#include "restclient-cpp/restclient.h"
#include "json.hpp"
#include "ConfigLoad.h"

using namespace std;
using json = nlohmann::json;

/*
 * Should make this a singleton at some point. Bad things might happen with init() and disable() otherwise.
 */
RestActions::RestActions() {
    RestClient::init();
    conn = new RestClient::Connection("https://cp-parking-process-api.run.aws-usw02-pr.ice.predix.io");
	RestClient::HeaderFields headers;
	headers["Content-Type"] = "application/json";
	conn->SetHeaders(headers);
}

RestClient::Connection* RestActions::getRestClient() {
	return conn;
}

RestClient::Response RestActions::postRequest(RestClient::Connection* conn, map<int, bool> data) 
{
	string postData = "{";
		postData += "\"ParkingLotId\": \"ParkingLot1\",";
        postData += "\"SensorId\": 
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
    return r;
}

bool RestActions::receiveConfiguration(RestClient::Response response, vector<Parking>* parking_data) {
    string raw = R"({
  "requireconfig" : true,
  "config" : [
    {
      "spotid" : 0,
      "coordinates" :
        {
          "x1" : 120,
          "y1" : 138,
          "x2" : 425,
          "y2" : 125,
          "x3" : 420,
          "y3" : 147,
          "x4" : 526,
          "y4" : 325
        }
    },
    {
      "spotid" : 1,
      "coordinates" : 
        {
          "x1" : 120,
          "y1" : 138,
          "x2" : 425,
          "y2" : 125,
          "x3" : 420,
          "y3" : 147,
          "x4" : 526,
          "y4" : 325
        }
    }
  ]
})";
    auto result = json::parse(raw);
    if (result["requireconfig"]) {
        parking_data->clear();

        for (int i = 0; i < result["config"].size(); i++) {
            int id = result["config"][i]["spotid"];
            int x1 = result["config"][i]["coordinates"]["x1"];
            int y1 = result["config"][i]["coordinates"]["y1"];
            int x2 = result["config"][i]["coordinates"]["x2"];
            int y2 = result["config"][i]["coordinates"]["y2"];
            int x3 = result["config"][i]["coordinates"]["x3"];
            int y3 = result["config"][i]["coordinates"]["y3"];
            int x4 = result["config"][i]["coordinates"]["x4"];
            int y4 = result["config"][i]["coordinates"]["y4"];
            //cout << x1 << y1 << x2 << y2 << x3 << y3 << x4 << y4 << endl;
            
            Parking space;
            vector<cv::Point> points;

            points.push_back(cv::Point(x1, y1));
		    points.push_back(cv::Point(x2, y2));
		    points.push_back(cv::Point(x3, y3));
		    points.push_back(cv::Point(x4, y4));
		    space.setId(id);
		    space.setPoints(points);
		    space.calcBoundingRect();

            parking_data->push_back(space);

        }
        cout << result["config"] << endl;
        cout << result["config"][0] << endl;
        return true;
    }
    
    return false;
}

RestActions::~RestActions() {
    RestClient::disable();
}