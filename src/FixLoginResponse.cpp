/*
 * FixLoginResponse.cpp
 *
 *  Created on: Mar 29, 2015
 *      Author: kdeol
 */

#include "FixLoginResponse.h"
#include <iostream>
using namespace v8;
using namespace node;
using namespace std;

FixLoginResponse::FixLoginResponse() : Nan::ObjectWrap(){

}

FixLoginResponse::~FixLoginResponse() {
}

void FixLoginResponse::Initialize(Local<Object> target) {
  Nan::HandleScope scope;

  Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>(FixLoginResponse::New);

  ctor->InstanceTemplate()->SetInternalFieldCount(1);
  ctor->SetClassName(Nan::New("FixLoginResponse").ToLocalChecked());

  Nan::SetPrototypeMethod(ctor, "done", done);

  Local<Context> context = Nan::GetCurrentContext();

  target->Set(context, Nan::New("FixLoginResponse").ToLocalChecked(), ctor->GetFunction(context).ToLocalChecked());
}

Local<Object> FixLoginResponse::wrapFixLoginResponse(FixLoginResponse* fixLoginResponse) {

	Local<FunctionTemplate> ctor = Nan::New<FunctionTemplate>();

	ctor->InstanceTemplate()->SetInternalFieldCount(1);
	ctor->SetClassName(Nan::New("FixLoginResponse").ToLocalChecked());

	Nan::SetPrototypeMethod(ctor, "done", done);

	Local<Object> obj = ctor->InstanceTemplate()->NewInstance(Nan::GetCurrentContext()).ToLocalChecked();
	//obj->SetInternalField(0, Nan::New<External>(fixLoginResponse));

	fixLoginResponse->Wrap(obj);
	fixLoginResponse->Ref();

	return obj;
}

NAN_METHOD(FixLoginResponse::New) {
	Nan::HandleScope scope;

	FixLoginResponse *loginResponse = new FixLoginResponse();

	loginResponse->Wrap(info.This());

	info.GetReturnValue().Set(info.This());
}

NAN_METHOD(FixLoginResponse::done) {
	Nan::HandleScope scope;

	FixLoginResponse* instance = Nan::ObjectWrap::Unwrap<FixLoginResponse>(info.This());

	bool success = info[0]->ToBoolean(Isolate::GetCurrent())->Value();

    instance->setIsFinished(true);
	instance->setIsLoggedOn(success);

	return;
}

bool FixLoginResponse::getIsFinished() {
	return isFinished;
}

bool FixLoginResponse::getIsLoggedOn() {
	return isLoggedOn;
}

void FixLoginResponse::setIsFinished(bool finished) {
	isFinished = finished;
}

void FixLoginResponse::setIsLoggedOn(bool loggedOn) {
	isLoggedOn = loggedOn;
}

void FixLoginResponse::close(FixLoginResponse* fixLoginResponse) {
	fixLoginResponse->Unref();
}

