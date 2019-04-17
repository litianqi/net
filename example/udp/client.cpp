#include <litianqi/net.hpp>

#if _WIN32
#include <winsock2.h>
#endif
#include <cstdio>
#include <iostream>

using namespace std;
using namespace litianqi::net;

int main() {
#if _WIN32
	{
		WSADATA data;
		int error = WSAStartup(MAKEWORD(2, 2), &data);
		if (error != 0) {
			printf("WSAStartup error: %d\n", WSAGetLastError());
			return -1;
		}
	}
#endif

	auto result = UdpSocket::bind(SocketAddrV4(Ipv4Addr::kLoopback, 0));
	if (auto error = get_if<Error>(&result)) {
		printf("bind error: %d\n", error->raw_os_error().value());
		return -1;
	}

	auto socket = get_if<UdpSocket>(&result);
	string input;
	while (getline(cin, input)) {
		auto result = socket->send_to(vector<uint8_t>(input.begin(), input.end()), SocketAddrV4(Ipv4Addr::kLoopback, 65535));
		if (auto error = get_if<Error>(&result)) {
			printf("send_to error: %d\n", error->raw_os_error().value());
			return -1;
		}
		printf("send: %s\n", input.c_str());
		input.clear();
	}

#if _WIN32
	{
		int error = WSACleanup();
		if (error != 0) {
			printf("WSACleanup error: %d\n", WSAGetLastError());
			return -1;
		}
	}
#endif
	return 0;
}