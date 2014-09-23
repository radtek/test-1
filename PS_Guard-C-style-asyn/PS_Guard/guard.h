#ifndef GUARD_H_
#define GUARD_H_
#include "node.h"
#include "v8.h"
#include "guardClient/GuardClient.h"
#include <dbghelp.h>
#pragma comment(lib, "DbgHelp")
using namespace node;
using namespace v8;
using namespace std;

#define REQ_FUN_ARG(I, VAR)                                                                          \
	if (args.Length() <= (I) || !args[I]->IsFunction())                                                \
	return ThrowException(Exception::TypeError(String::New("Argument " #I " must be a function")));  \
	Local<Function> VAR = Local<Function>::Cast(args[I]);

Local<Function> cb;
struct Baton{
	bool isStop;
	Local<Function> cb;
}baton={false,cb};


#endif