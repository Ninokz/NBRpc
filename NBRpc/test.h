#pragma once
#include "rpcclientstub.h"
#include "rpcserverstub.h"
#include "rpcprocedure.h"

#include <json/json.h>

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