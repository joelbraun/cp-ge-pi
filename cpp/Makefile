all:
	g++ -Wall -g -std=c++11 `pkg-config --cflags opencv` ./*.cpp `pkg-config --libs opencv` -l restclient-cpp -lcurl  -o ./detect-parking
clean:
	rm -rf *.o
