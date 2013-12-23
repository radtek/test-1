#include <node.h>
#include <v8.h>
#include "person.h"
#include "animal.h"

using namespace v8;



extern "C" {
	static void init(Handle<Object> target) {
		Person::init(target);
		Animal::init(target);
	}

	NODE_MODULE(classTest, init);
}