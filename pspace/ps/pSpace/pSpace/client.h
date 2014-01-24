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

class Client : ObjectWrap {
public:
  static void init(Handle<Object> target);
  static Handle<Value> New(const Arguments& args);
  static Handle<Value> connect(const Arguments& args);
  static Handle<Value> connectSync(const Arguments& args);
  static void connectWork(uv_work_t* req);
  static void afterConnect(uv_work_t* req, int status);
 
  PSHANDLE getHandle();
  static void setHandle(PSHANDLE h);

  Client(){}
  ~Client(){}

private:
  static Persistent<FunctionTemplate> constructTemplate_;
  static  PSHANDLE handle_;
};


#endif