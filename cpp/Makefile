all: detect-parking
	g++ -Wall -g -std=c++11 -stdlib=libc++ `pkg-config --cflags --libs opencv` -l restclient-cpp ./*.cpp -o ./detect-parking
clean:
	rm -rf *.o
