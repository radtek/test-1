#ifndef COMMON_H_
#define COMMON_H_

#include <v8.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sstream>
#include <iostream>
#include "psAPISDK.h"

using namespace v8;

//将node.js类型字符串转化为C++可用字符串
const char* ToCString(const String::Utf8Value& value);

//把PS_VARIANT 转换为DOUBLE字符串
double PS_VARIANT2DOUBLE(const PS_VARIANT &var);
#endif