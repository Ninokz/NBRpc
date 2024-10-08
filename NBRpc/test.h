#pragma once
#include "log.h"
#include "rpcclientstub.h"
#include "rpcserverstub.h"
#include "rpcprocedure.h"

#include <json/json.h>


void InitLogger()
{
	const char* consoleDetailParrten = "%d{%Y-%m-%d %H:%M:%S}%T[%p]%T[%c]%T%t%T%N%T%f:%l%T%m%n";
	const char* consoleStdParrten = "[%p]%T%d{%Y-%m-%d %H:%M:%S}%T%t%T%N%T%m%n";
	auto stdlogger = ASYNC_LOG_NAME("STD_LOGGER");
	Nano::Log::LogFormatter::ptr stdFormat = std::make_shared<Nano::Log::LogFormatter>(consoleStdParrten);
	Nano::Log::ANSIColorStdoutLogAppender::ptr stdAppender = std::make_shared<Nano::Log::ANSIColorStdoutLogAppender>();
	stdAppender->setFormatter(stdFormat);
	stdlogger->addAppender(stdAppender);
	stdlogger->setLevel(Nano::Log::LogLevel::Level::DEBUG);
}

/// hello world test case once

void helloworldReturnService(Json::Value& request, const Nano::Rpc::ProcedureDoneCallback& done) {
	Json::Value result = "Hello, " + request["params"]["name"].asString() + "!";
	
	bool flag = false;
	Nano::JrpcProto::JsonRpcResponse::Ptr response = Nano::JrpcProto::JsonRpcResponseFactory::createResponseFromRequest(request, result, &flag);
	done(response->toJson());
}


void RpcServerStubHelloWorldTest() {
	Nano::Rpc::RpcServerStub::Ptr rpcServerStub = std::make_shared<Nano::Rpc::RpcServerStub>(9800);
	std::unordered_map<std::string, Json::ValueType> paramsNameTypesMap = {
	  {"name", Json::ValueType::stringValue}
	};
	rpcServerStub->registReturn("helloworldMethod", paramsNameTypesMap, helloworldReturnService);
	rpcServerStub->run();
	system("pause");
	rpcServerStub->stop();
}

void helloworldCallback(Json::Value response) {
	std::cout << response.toStyledString() << std::endl;
};

void ClientStubHelloWorldTest() {
	std::unordered_map<std::string, Json::Value> params = {
	  {"name", "World"}
	};
	auto result = Nano::Rpc::RpcClientOnceStub::rpcReturnCallOnce("127.0.0.1", 9800, "1", "helloworldMethod", params, helloworldCallback, 3000);
	std::cout << result->response->getResult().asString() << std::endl;
}

void highConcurrencyClientStubHelloWorldTest() {
	std::unordered_map<std::string, Json::Value> params = {
	  {"name", "World"}
	};
	auto client = std::make_shared<Nano::Rpc::RpcClientStub>();
	client->connect("127.0.0.1", 9800);
	for (int i = 0; i < 100; i++) {
		std::string id = std::to_string(i);
		std::cout << id << "send" << std::endl;
		auto result = client->asyncRpcReturnCall(id, "helloworldMethod", params, helloworldCallback, 1000);
	}
	auto end = std::chrono::high_resolution_clock::now();
	system("pause");
	client->disconnect();
}

//// Substract Test Case once

void substractReturnService(Json::Value& request, const Nano::Rpc::ProcedureDoneCallback& done) {
	int subtrahend = request["params"]["subtrahend"].asInt();
	int minuend = request["params"]["minuend"].asInt();
	Json::Value result = minuend - subtrahend;

	bool flag = false;
	Nano::JrpcProto::JsonRpcResponse::Ptr response = Nano::JrpcProto::JsonRpcResponseFactory::createResponseFromRequest(request, result, &flag);
	done(response->toJson());
}

void RpcServerStubSubstractTest() {
	Nano::Rpc::RpcServerStub::Ptr rpcServerStub = std::make_shared<Nano::Rpc::RpcServerStub>(9800);
	std::unordered_map<std::string, Json::ValueType> paramsNameTypesMap = {
	  {"subtrahend", Json::ValueType::intValue},
	  {"minuend", Json::ValueType::intValue}
	};

	rpcServerStub->registReturn("substractMethod", paramsNameTypesMap, substractReturnService);
	rpcServerStub->run();
	system("pause");
	rpcServerStub->stop();
}

void substractCallback(Json::Value response) {
	std::cout << response.toStyledString() << std::endl;
};

void ClientStubSubstractTest() {
	std::unordered_map<std::string, Json::Value> params = {
	  {"subtrahend", 23},
	  {"minuend", 42}
	};
	auto result = Nano::Rpc::RpcClientOnceStub::rpcReturnCallOnce("127.0.0.1", 9800, "1", "substractMethod", params, substractCallback, 3000);
	std::cout << result->response->getResult().asString() << std::endl;
}

/// hello world notify test case once

void ClientStubhelloNotifyTest() {
	Nano::Rpc::RpcClientStub::Ptr rpcClientStub = std::make_shared<Nano::Rpc::RpcClientStub>();
	std::unordered_map<std::string, Json::Value> params = {
	  {"notify", "World"}
	};
	auto result = Nano::Rpc::RpcClientOnceStub::rpcNotifyCallOnce("127.0.0.1", 9800, "helloworldMethod", params);
	std::cout << "notify success :" << result << std::endl;
}

void helloNotifyService(Json::Value& request)
{
	
}

void RpcServerStubhelloNotifyTest() {
	Nano::Rpc::RpcServerStub::Ptr rpcServerStub = std::make_shared<Nano::Rpc::RpcServerStub>(9800);
	std::unordered_map<std::string, Json::ValueType> paramsNameTypesMap = {
	  {"notify", Json::ValueType::stringValue}
	};
	rpcServerStub->registNotify("helloNotifyMethod", paramsNameTypesMap, helloNotifyService);
	rpcServerStub->run();
	system("pause");
	rpcServerStub->stop();
}

