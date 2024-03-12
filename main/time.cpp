#include<iostream>
#include<memory>
#include<chrono>

class Timer {
public:
	Timer() {
		mstart = std::chrono::high_resolution_clock::now();
	}
	~Timer() {
		Stop();
	}
	void Stop() {
		auto endtime = std::chrono::high_resolution_clock::now();
		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(mstart).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endtime).time_since_epoch().count();
		auto duration = end - start;
		double ms = duration * 0.001;
		std::cout<<ms<<"ms\n";
	}
	void count() {
		auto endtime = std::chrono::high_resolution_clock::now();
		auto start = std::chrono::time_point_cast<std::chrono::microseconds>(mstart).time_since_epoch().count();
		auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endtime).time_since_epoch().count();
		auto duration = end - start;
		double ms = duration * 0.001;
		std::cout << ms << "ms\n";
	}
private:
	std::chrono::time_point<std::chrono::high_resolution_clock>mstart;
};