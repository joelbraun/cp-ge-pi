all:
	g++ -Wall -g -std=c++11 `pkg-config --cflags opencv` ./*.cpp `pkg-config --libs opencv` -l restclient-cpp -lcurl -I/usr/local/include/ -L/opt/vc/lib  -lraspicam -lraspicam_cv -lmmal -lmmal_core -lmmal_util -o ./detect-parking
clean:
	rm -rf *.o
