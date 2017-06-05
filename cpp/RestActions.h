#include <vector>
#include "Parking.h"
#include "restclient-cpp/connection.h"
#include "restclient-cpp/restclient.h"

using namespace std;

class RestActions {
private:
    RestClient::Connection* conn;
    string processToken;
    string configureToken;
public:
    RestActions();
    RestClient::Response postRequest(RestClient::Connection* conn, map<int,bool> status);
    bool receiveConfiguration(RestClient::Response response, vector<Parking> *parking_data);
    RestClient::Connection* getRestClient();
    void getProcessOAuthToken();
    void getConfigOAuthToken();
    ~RestActions();
};
