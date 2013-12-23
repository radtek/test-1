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

//��node.js�����ַ���ת��ΪC++�����ַ���
const char* ToCString(const String::Utf8Value& value);

//��PS_VARIANT ת��ΪDOUBLE�ַ���
double PS_VARIANT2DOUBLE(const PS_VARIANT &var);
#endif