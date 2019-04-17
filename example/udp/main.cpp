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
			printf("WSAStartup error: %d", WSAGetLastError());
			return -1;
		}
	}
#endif

	Ipv4Addr ip{ 0, 0, 0, 0 };
	auto result = UdpSocket::bind(SocketAddrV4{ ip, 49152 });
	if (auto error = get_if<Error>(&result)) {
		printf("bind error: %d", error->raw_os_error().value());
		return -1;
	}

	auto socket = get_if<UdpSocket>(&result);
	vector<uint8_t> buf; buf.reserve(65536);
#if CLIENT
	string input;
	while (getline(cin, input)) {

		auto result = socket->send_to(vector<uint8_t>(input.begin(), input.end()), SocketAddrV4{ Ipv4Addr{ 127, 0, 0, 1 }, PORT_OTHER });
		if (auto error = get_if<Error>(&result)) {
			printf("send_to error: %d", error->raw_os_error().value());
			return -1;
		}
		printf("send: %s", input.c_str());
		input.clear();
#endif
#if SERVER
		auto result = socket->recv_from(buf);
		if (auto error = get_if<Error>(&result)) {
			printf("recv_from error: %d", error->raw_os_error().value());
			return -1;
		}
		size_t size = get<0>(get<tuple<size_t, SocketAddrV4>>(result));
		SocketAddrV4 addr = get<1>(get<tuple<size_t, SocketAddrV4>>(result));
		printf("recv: %s", string(buf.begin(), buf.begin() + size).c_str());
		buf.clear();
#endif
	}

#if _WIN32
	{
		int error = WSACleanup();
		if (error != 0) {
			printf("WSACleanup error: %d", WSAGetLastError());
			return -1;
		}
	}
#endif
	return 0;
}