/*
 * Copyright 2019 Tianqi Li. All rights reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "litianqi/net.hpp"

#ifdef _WIN32
#include <WinSock2.h>
#include <Ws2tcpip.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <stdexcept>

namespace litianqi::net {

	const Ipv4Addr Ipv4Addr::kAny = INADDR_ANY;

	const Ipv4Addr Ipv4Addr::kBroadcast = INADDR_BROADCAST;

	const Ipv4Addr Ipv4Addr::kLoopback = INADDR_LOOPBACK;

	uint8_t& Ipv4Addr::operator[](int index)
	{
		if (index < 0 || index > 3)
			throw std::out_of_range("index is out of range.");
		return ((uint8_t*)& ip_)[4 - index];
	}

	sockaddr_in to_os_addr(const SocketAddrV4 & addr) {
		sockaddr_in os_addr{};
		os_addr.sin_family = AF_INET;
		os_addr.sin_addr.S_un.S_addr = htonl(addr.ip());
		os_addr.sin_port = htons(addr.port());
		return os_addr;
	}

	SocketAddrV4 from_os_addr(sockaddr_in os_addr) {
		int ip = ntohl(os_addr.sin_addr.S_un.S_addr);
		uint16_t port = ntohs(os_addr.sin_port);
		return SocketAddrV4(Ipv4Addr(ip), port);
	}

	Error Error::last_os_error()
	{
#ifdef _WIN32
		int code = WSAGetLastError();
#else
		int code = errno;
#endif
		return from_os_error(code);
	}

	Error Error::from_os_error(int code)
	{
		Error error{ ErrorKind::Other };
		error.code_ = code;
		return error;
	}

	struct UdpSocket::Impl {
#if _WIN32
		SOCKET socket;
#else
		int socket;
#endif
		std::array<char, 65536> buf;
	};

	std::variant<UdpSocket, Error> UdpSocket::bind(const SocketAddrV4 & addr)
	{
#if _WIN32
		SOCKET sock = ::socket(AF_INET, SOCK_DGRAM, 0);
#else
		int sock = ::socket(AF_INET, SOCK_DGRAM, 0);
#endif
		sockaddr_in os_addr = to_os_addr(addr);
		int error = ::bind(sock, (sockaddr*)(&os_addr), sizeof(sockaddr_in));
		if (error != 0) {
			return Error::last_os_error();
		}
		UdpSocket socket{};
		socket.impl_->socket = sock;
		return socket;
	}

	std::variant<std::tuple<std::vector<uint8_t>, SocketAddrV4>, Error> UdpSocket::recv_from()
	{
		sockaddr_in os_addr{};
		int os_addr_len = sizeof(sockaddr_in);
		int len = ::recvfrom(impl_->socket, impl_->buf.data(), (int)impl_->buf.size(), 0, (sockaddr*)& os_addr, &os_addr_len);
		if (len < 0) {
			return Error::last_os_error();
		}
		return std::tuple{ std::vector<uint8_t>(impl_->buf.begin(), impl_->buf.begin() + len), from_os_addr(os_addr) };
	}

	std::variant<size_t, Error> UdpSocket::send_to(const std::vector<uint8_t> & buf, const SocketAddrV4 & addr)
	{
		sockaddr_in os_addr = to_os_addr(addr);
		int len = ::sendto(impl_->socket, (char*)buf.data(), (int)buf.size(), 0, (sockaddr*)& os_addr, (int)sizeof(sockaddr_in));
		if (len < 0) {
			return Error::last_os_error();
		}
		return len;
	}

	std::optional<Error> UdpSocket::set_nonblocking(bool nonblocking)
	{
#if _WIN32
		u_long arg = nonblocking ? 1 : 0;
		int error = ioctlsocket(impl_->socket, FIONBIO, &arg);
#else
		int flags = fcntl(socket_, F_GETFL, 0);
		int error = fcntl(socket_, F_SETFL, nonblocking ? (flags | O_NONBLOCK) : (flags & ~O_NONBLOCK));
#endif
		if (error != 0) {
			return Error::last_os_error();
		}
		return std::nullopt;
	}

	UdpSocket::UdpSocket()
		: impl_(std::make_shared<Impl>())
	{
	}
}  // namespace net