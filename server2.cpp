/*
I'm working on extending this to be a web application and API

*/
#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include <memory>
#include <sstream>
#include <fstream>

using namespace std;


class Session : public enable_shared_from_this<Session>{
public:
	Session(boost::asio::ip::tcp::socket socket) : socket_(move(socket)) {}

	void start() {
		read_req();

	}

private:
	void read_req(){
		auto self(shared_from_this());
		boost::asio::async_read_until(socket_, request_, "\r\n\r\n",[this, self](boost::system::error_code ec,size_t bytes_transferred) {
			if(!ec){
				handle_req(bytes_transferred);
			}
		});
	}

	void handle_req(size_t bytes_transferred) {
		string request_str(boost::asio::buffer_cast<const char*>(request_.data()), bytes_transferred);
		request_.consume(bytes_transferred); //remove consumed data

		cout << "Reaquest: " << request_str << endl;


		//check request route
		if ( request_str.find("/main") != string:: npos) {
			//send respons
			string response = "HTTP/1.1 200 OK\r\n\r\nHello World";
			boost::asio::async_write(socket_,boost::asio::buffer(response),[this](boost::system::error_code ec,size_t /*bytes_transeferred*/){
				if (ec){
					cout << "Error sending response: " << ec.message() << endl;
				}
			});

		} else if ( request_str.find("/") != string:: npos) {
			//send respons
			string response = "HTTP/1.1 200 OK\r\n"
							   "Content-Type: text/html; charset=UTF-8\r\n"
							   "Connection: close\r\n"
							   "\r\n"
							   "<!DOCTYPE html>"
							   "<html>"
							   "<body>"
							   "<h1>Hello From C++</h><br>"
							   "<input type=text placeholder='Enter your name' style='border-radius:9px;font-size:20px;border-wigth:2px;border-style:solid;border-color:black'>"
							   "</body>"
							   "</html>";


			boost::asio::async_write(socket_,boost::asio::buffer(response),[this](boost::system::error_code ec,size_t /*bytes_transeferred*/){
				if (ec){
					cout << "Error sending response: " << ec.message() << endl;
				}
			});

		} else {
			//send 404 response
			string response = "HTTP/1.1 404 Not Fount\r\n\r\n";
			boost::asio::async_write(socket_,boost::asio::buffer(response),[this](boost::system::error_code ec, size_t/*bytes_transferred*/){
				if (ec){
					cout << "Error sending response: " << ec.message() << endl;
				}
			});
		}
	}

	boost::asio::ip::tcp::socket socket_;
	boost::asio::streambuf request_;

};


class Server {
public:
	Server(boost::asio::io_service& io_service, short port)
		: acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)){
			start_accept();
		}

private:
	void start_accept(){
		acceptor_.async_accept(
			[this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket) {
				if (!ec) {
					//create a new session for the connected client
					make_shared<Session>(move(socket))->start();
				}
				//start accepting new connections
				start_accept();
			});
		}
		boost::asio::ip::tcp::acceptor acceptor_;
};


//Main function

int main() {

	try{

		boost::asio::io_service io_service;
		Server server(io_service, 80);
		cout << "Server nunning port: 80" << endl;
		io_service.run(); //start the asynchronous event loop

	} catch (exception& e) {

		cout << "Exception: " << e.what() << endl;
	}

	return 0;
}
