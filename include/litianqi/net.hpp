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

#pragma once
#include <cassert>
#include <variant>
#include <optional>
#include <string>
#include <array>
#include <vector>
#include <tuple>
#include <memory>

namespace litianqi::net {

	// A list specifying general categories of I/O error.
	//
	// This list is intended to grow over time and it is not recommended to exhaustively match against it.
	enum class ErrorKind {
		// TODO: Add more kinds.
		// Any I / O error not part of this list.
		Other
	};

	// The error type for I/O operations.
	//
	// Errors mostly originate from the underlying OS, but custom instances of Error can be created with a particular value of ErrorKind.
	class Error {
	public:
		// Returns an error representing the last OS error which occurred.
		//
		// This function reads the value of errno for the target platform (e.g. GetLastError on Windows) and will return a corresponding instance of Error for the error code.
		static Error last_os_error();

		// Creates a new instance of an Error from a particular OS error code.
		static Error from_os_error(int code);

		// Creates a new I/O error from a known kind of error.
		//
		// This function is used to generically create I/O errors which do not originate from the OS itself.
		Error(ErrorKind kind) : kind_(kind) {}

		// Returns an error representing the last OS error which occurred.
		//
		// If this Error was constructed via last_os_error or from_raw_os_error, then this function will return int, otherwise it will return empty.
		std::optional<int> raw_os_error() const { return code_; }

		// Returns the corresponding ErrorKind for this error.
		ErrorKind kind() const { return kind_; }
	private:
		ErrorKind kind_;
		std::optional<int> code_;
	};

	// An IPv4 address.
	class Ipv4Addr {
	public:
		// An any address (0.0.0.0).
		static const Ipv4Addr kAny;

		// A broadcast address (255.255.255.255).
		static const Ipv4Addr kBroadcast;

		// A loopback address (127.0.0.1).
		static const Ipv4Addr kLoopback;

		// Creates a new IPv4 address from four eight-bit octets.
		Ipv4Addr(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
			: ip_(((uint32_t)d << 24) + ((uint32_t)c << 16) + ((uint32_t)b << 8) + (uint32_t)a) {}

		// Creates a new IPv4 address from an 32-bit integer.
		Ipv4Addr(uint32_t ip) : ip_(ip) {}

		// Returns true if this is a broadcast address (255.255.255.255).
		bool is_broadcast() const {
			return *this == Ipv4Addr::kBroadcast;
		}

		// Returns true if this is a loopback address (127.0.0.0/8).
		bool is_loopback() const {
			return (*this)[0] == 127 && (*this)[1] == 0 && (*this)[2] == 1;
		}

		// Compares the octets of two addresses.
		bool operator==(const Ipv4Addr& other) const {
			return ip_ == other.ip_;
		}

		// Returns the eight-bit octet on the specified index.
		uint8_t& operator[](int index);

		// Returns the eight-bit octet on the specified index.
		const uint8_t& operator[](int index) const { return operator[](index); }

		// Implicitly converted to an uint32_t integer.
		operator uint32_t() { return ip_; }

		// Implicitly converted to an uint32_t integer.
		operator uint32_t() const { return ip_; }

	private:
		uint32_t ip_ = 0;
	};

	// An IPv4 socket address.
	class SocketAddrV4 {
	public:
		// Creates a new socket address from an IPv4 address and a port number.
		SocketAddrV4(Ipv4Addr ip, uint16_t port) : ip_(ip), port_(port) {}

		// Returns the IP address associated with this socket address.
		const Ipv4Addr& ip() const { return ip_; }

		// Changes the IP address associated with this socket address.
		void set_ip(Ipv4Addr ip) { ip_ = ip; }

		// Returns the port number associated with this socket address.
		uint16_t port() const { return port_; }

		// Changes the port number associated with this socket address.
		void set_port(uint16_t port) { port_ = port; }

	private:
		Ipv4Addr ip_;
		uint16_t port_;
	};

	// A UDP socket.
	//
	// After creating a UdpSocket by binding it to a socket address, data can be sent to and received from any other socket address.
	class UdpSocket {
	public:
		// Creates a UDP socket from the given address.
		static std::variant<UdpSocket, Error> bind(const SocketAddrV4& addr);

		// Receives a single datagram message on the socket. On success, returns the number of bytes read and the origin.
		//
		// The function must be called with valid byte array buf of sufficient size to hold the message bytes. If a message 
		// is too long to fit in the supplied buffer, excess bytes may be discarded.
		std::variant<std::tuple<std::vector<uint8_t>, SocketAddrV4>, Error> recv_from();

		// Sends data on the socket to the given address. On success, returns the number of bytes written.
		std::variant<size_t, Error> send_to(const std::vector<uint8_t>& buf, const SocketAddrV4& addr);

		// Moves this UDP socket into or out of nonblocking mode.
		std::optional<Error> set_nonblocking(bool nonblocking);
	private:
		struct Impl;
		UdpSocket();
		std::shared_ptr<Impl> impl_;
	};
}  // namespace net
