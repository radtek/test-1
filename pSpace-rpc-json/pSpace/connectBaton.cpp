#include  <cstring>
#include "connectBaton.h"

ConnectBaton::ConnectBaton(Client* client,v8::Handle<v8::Function>* callback)
{
	this->client_ = client;
	
	if(callback != NULL) {
		this->callback_ = Persistent<Function>::New(*callback);
	} else {
		this->callback_ = Persistent<Function>();
	}
	this->handle_ = PSHANDLE_UNUSED;
	this->hostname_ = "127.0.0.1";
	this->user_ = "";
	this->password_ = "";
	this->port_ = 8889;

	this->code_ = PSRET_OK;
	//strcpy(errString_,"");
	this->errString_ = PSNULL;
	//this->e = new ErrObj();
	
}

ConnectBaton::~ConnectBaton() {
	callback_.Dispose();
    if (this->errString_ != PSNULL)
    {
        delete this->errString_;
        this->errString_ = PSNULL;
    }
    
}