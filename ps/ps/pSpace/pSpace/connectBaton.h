#ifndef CONNECT_BATON_H_
#define CONNECT_BATON_H_

#include <node.h>
#include <v8.h>
#include <string>
#include "psAPISDK.h"
#include "psAPICommon.h"
#include "psException.h"

using namespace node;
using namespace v8;

class Client;
class ConnectBaton {
public:
	ConnectBaton(Client* client, PSHANDLE h, v8::Handle<v8::Function>* callback);
	~ConnectBaton();
	Client* client_;
	Persistent<Function> callback_;
	std::string hostname_;
	std::string user_;
	std::string password_;
	uint32_t port_;
	PSHANDLE handle_;
	
	int code_;
	char errString_[70];
};

#endif