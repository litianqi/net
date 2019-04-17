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

	auto result = UdpSocket::bind(SocketAddrV4(Ipv4Addr::kLoopback, 65535));
	if (auto error = get_if<Error>(&result)) {
		printf("bind error: %d\n", error->raw_os_error().value());
		return -1;
	}

	auto socket = get_if<UdpSocket>(&result);
	vector<uint8_t> buf(65536, 0);
	while (true) {
		auto result = socket->recv_from();
		if (auto error = get_if<Error>(&result)) {
			printf("recv_from error: %d\n", error->raw_os_error().value());
			return -1;
		}
		vector<uint8_t> bytes = get<0>(get<0>(result));
		SocketAddrV4 addr = get<1>(get<0>(result));
		printf("recv: %s\n", string(bytes.begin(), bytes.end()).c_str());
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