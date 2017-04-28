#ifndef CLIENT_H_
#define CLIENT_H_

#include <node.h>
#include <v8.h>
#include <node_object_wrap.h>
#include <cstring>
#include "psAPISDK.h"
#include "utils.h"
#include "pSpaceNode.h"
#include "connectBaton.h"

class PsException;

using namespace node;
using namespace v8;
using namespace std;
using v8::Isolate;

class Client : ObjectWrap {
public:
  static void init(Handle<Object> target);
  static void New(const FunctionCallbackInfo<Value>& args);
  static void connect(const FunctionCallbackInfo<Value>& args);
  static void isConnected(const FunctionCallbackInfo<Value>& args);
  static void connectSync(const FunctionCallbackInfo<Value>& args);
  PSHANDLE getHandle();
private:
  static void connectWork(uv_work_t* req);
  static void afterConnect(uv_work_t* req, int status);
 
  
  //static void setHandle(PSHANDLE h);

  Client():handle_(PSHANDLE_UNUSED){}
  ~Client()
  {
	  constructor_.Reset();
  }

private:
  static Persistent<Function> constructor_;
  PSHANDLE handle_;
};


#endif