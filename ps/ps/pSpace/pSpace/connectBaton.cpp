#include  <cstring>
#include "connectBaton.h"

ConnectBaton::ConnectBaton(Client* client, PSHANDLE h, v8::Handle<v8::Function>* callback)
{
	this->client_ = client;
	
	if(callback != NULL) {
		this->callback_ = Persistent<Function>::New(*callback);
	} else {
		this->callback_ = Persistent<Function>();
	}
	this->handle_ = h;
	this->hostname_ = "127.0.0.1";
	this->user_ = "";
	this->password_ = "";
	this->port_ = 8889;

	this->code_ = PSRET_OK;
	strcpy(errString_,"");
	
	
}

ConnectBaton::~ConnectBaton() {
	callback_.Dispose();
}