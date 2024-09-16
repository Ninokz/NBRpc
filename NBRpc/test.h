#pragma once
#include "rpcclientstub.h"
#include "rpcserverstub.h"
#include "rpcprocedure.h"

#include <json/json.h>

/// hello world test case

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
	
};

void ClientStubHelloWorldTest() {
	Nano::Rpc::RpcClientStub::Ptr rpcClientStub = std::make_shared<Nano::Rpc::RpcClientStub>();
	std::unordered_map<std::string, Json::Value> params = {
	  {"name", "World"}
	};
	rpcClientStub->rpcReturnCall("127.0.0.1", 9800, "1", "helloworldMethod", params, helloworldCallback, 3000);
	system("pause");
}

//// Substract Test Case

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
	
};

void ClientStubSubstractTest() {
	Nano::Rpc::RpcClientStub::Ptr rpcClientStub = std::make_shared<Nano::Rpc::RpcClientStub>();
	std::unordered_map<std::string, Json::Value> params = {
	  {"subtrahend", 23},
	  {"minuend", 42}
	};
	rpcClientStub->rpcReturnCall("127.0.0.1", 9800, "1", "substractMethod", params, substractCallback, 3000);
}

/// hello world notify test case

void ClientStubhelloNotifyTest() {
	Nano::Rpc::RpcClientStub::Ptr rpcClientStub = std::make_shared<Nano::Rpc::RpcClientStub>();
	std::unordered_map<std::string, Json::Value> params = {
	  {"notify", "World"}
	};
	rpcClientStub->rpcNotifyCall("127.0.0.1", 9800, "helloworldMethod", params);
	system("pause");
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

