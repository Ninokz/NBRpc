# NBRpc

NBRpc 是使用 `C++ 14` 编写的基于 `Boos.Asio` 异步 RPC 框架，通信数据传输用了自己简单在`Boos.Asio`上封装的网络库，端间采用 `json` 进行 `request` 与 `response` 传输，消息传输协议采用 [JSON-RPC 2.0](https://www.jsonrpc.org/specification) 规范. 框架结构如下图

![](https://observer-storage.oss-cn-chengdu.aliyuncs.com/github/Nano/nano-rpcdesign.png)

框架底层网络库采用 [Boost](https://www.boost.org/users/history/version_1_85_0.html) 的 `Asio` 库，其支持多平台，理论上`Nano Frameworks` 框架也能在 Linux 上编译，只不过目前建立的是一个 Visual Studio 的工程，此外这个框架还支持异步日志、多种类型线程池选用等常见的功能。

`BaseServer`设计的核心部分主要是一个连接监听器`acceptor`和一个`IOServicePool`，前者用于监听连接请求构建`session`，将连接的`socket`部署到`session`的`io_context`后再将`io_context`注册到后者中，后者`IOServicePool`本质上是一个线程池，只不过每个线程管理不同`session`的`io_context`，并发处理不同`io_context`读写事件，`RPCServer`设计的核心部分主要是一个线程池用于执行`RPCClient`发起的远程调用请求。

## Dependency.

- [Boost 1.85.0](https://www.boost.org/users/history/version_1_85_0.html)
- [jsoncpp 1.9.5](https://github.com/open-source-parsers/jsoncpp)

## Use Example.

注册服务：构建一个 `RpcServerStub::Pre` 对象，只需要调用 `RpcServerStub::Pre` 对象的 `registReturn` 方法进行注册 RPC 服务，通过 `registReturn` 方法的  `std::unordered_map` 参数设定 RPC 所需的参数类型和名称；然后再定义 RPC 方法本身，通过定义符合 `std::function<void(Json::Value&, const RpcDoneCallback&)>` 类型的可调用对象即可

运行服务：使用上述构建的 `RpcServerStub::Pre` 对象，运行即可；在运行之前若需要输出控制台信息，调用 `InitLoggers()` 

请求服务：构建一个 `RpcClientStub::Ptr` 对象，只需要调用 `RpcClientStub::Ptr` 对象的 `rpcReturnCall` 或者 `asyncRpcReturnCall` 方法或者 `rpcNotifyCall` 方法即可，只不过需要传入远端 IP 和 端口以及对应 RPC 参数；

示例 - HelloWorld Service

服务端注册服务

```c++
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
```

客户端请求服务：

```C++
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
```

## Ref.

- [NBLib](https://github.com/Ninokz/NBlib): Net lib based on boost.asio
- [libjson-rpc-cpp](https://github.com/cinemast/libjson-rpc-cpp): C++ framework for json-rpc
- [jrpc](https://github.com/guangqianpeng/jrpc): JSON-RPC implementation in C++17
- [llfc blog](https://www.llfc.club/category?catid=225RaiVNI8pFDD5L4m807g7ZwmF) 