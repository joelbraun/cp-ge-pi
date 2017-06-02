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
#include "base64.h"

using namespace std;
using json = nlohmann::json;

/*
 * Should make this a singleton at some point. Bad things might happen with init() and disable() otherwise.
 */
RestActions::RestActions() {
    RestClient::init();
    conn = new RestClient::Connection("http://process.parkix.io");
	RestClient::HeaderFields headers;
	headers["Content-Type"] = "application/json";
	conn->SetHeaders(headers);
}

RestClient::Connection* RestActions::getRestClient() {
	return conn;
}

void RestActions::getOAuthToken()
{
    string url = "https://a7cc7c64-503e-43df-ab3a-18c97ef60505.predix-uaa.run.aws-usw02-pr.ice.predix.io";
    string clientId = "sensor_client";
    string clientSecret = "I76MWuYSQ9Am99DFlMRSbcixGenRx0GOrAowx06hGMZvAb9E1gJfNIn1dwBUD1GIGu4TauVuYYF2buZur7b0BPTGlPfL2RtpRsgvRgrCeKvvCnZ8L4msd5lAawhkBpyO";
    RestClient::Connection *oAuthConn = new RestClient::Connection(url);
    RestClient::HeaderFields headers;
    string preBase64Creds = clientId + ":" + clientSecret;
    string postBase64Creds; 
    Base64::Encode(preBase64Creds,&postBase64Creds); 
    cout << postBase64Creds << endl;
    headers["authorization"] = "Basic " + postBase64Creds;
    headers["content-type"] = "application/x-www-form-urlencoded";
    string postData = "client_id=sensor_client&grant_type=client_credentials";
    oAuthConn->SetHeaders(headers);
    RestClient::Response r = oAuthConn->post("/oauth/token", postData);
    auto result = json::parse(r.body);
    token = result["access_token"];   
    cout << token << endl;
    conn->AppendHeader("authorization", token);    
}

RestClient::Response RestActions::postRequest(RestClient::Connection* conn, map<int, bool> data) 
{
	string postData = "{";
		postData += "\"sensorId\": \"PI1\",";
        postData += "\"SensorId\":"; 
		postData += "\"Timestamp\": \"";
		char time_buf[21];
		time_t now;
		time(&now);
	    strftime(time_buf, 21, "%Y-%m-%dT%H:%S:%MZ", localtime(&now));
		postData += time_buf;
		postData += "\",";
		postData += "\"spotsTaken\": ";
        int ct = 0; 
	for (int i = 0; i < data.size(); i++) {
              if (data[i])
              {
              ct++;
              }
        }
	postData += to_string(ct) + ",";
        postData += "\"licensePlates\":[""]}";
	cout << postData << endl;                                      
	RestClient::Response r = conn->post("/api/ingest", postData);
	cout << "DID POST SUCCEED" << r.code << endl;
    return r;
}

bool RestActions::receiveConfiguration(RestClient::Response response, vector<Parking>* parking_data) {
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
auto result = json::parse(raw);		    points.push_back(cv::Point(x2, y2));
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
