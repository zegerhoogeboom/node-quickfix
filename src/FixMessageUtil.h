/*
 * FixMessageUtil.h
 *
 *  Created on: Sep 8, 2014
 *      Author: kdeol
 */

#ifndef FIXMESSAGEUTIL_H_
#define FIXMESSAGEUTIL_H_

#include <v8.h>
#include <node.h>
#include <nan.h>

#include "FixEvent.h"
#include "quickfix/SessionID.h"
#include "quickfix/Message.h"

using namespace node;

class FixMessageUtil {
public:
	FixMessageUtil();
	virtual ~FixMessageUtil();

	static void addFixHeader(FIX::Message* message, Local<v8::Object> msg) {

		Local<v8::Object> header = Local<v8::Object>::Cast(msg->Get(Nan::GetCurrentContext(), Nan::New<String>("header").ToLocalChecked()).ToLocalChecked());
		FIX::Header &msgHeader = message->getHeader();
		Local<v8::Array> headerTags = header->GetPropertyNames(Nan::GetCurrentContext()).ToLocalChecked();

		for(int i=0; i < (int)headerTags->Length(); i++) {
            String::Utf8Value value(Isolate::GetCurrent(), Nan::To<v8::String>(
                    header->Get(Nan::GetCurrentContext(),
                                headerTags->Get(Nan::GetCurrentContext(), i).ToLocalChecked()
                                ).ToLocalChecked()).ToLocalChecked());
            msgHeader.setField(
                    Nan::To<int32_t>(headerTags->Get(Nan::GetCurrentContext(), i).ToLocalChecked()).ToChecked(),
					std::string(*value)
			);
		}
	}

	static void addFixTags(FIX::FieldMap* map, Local<v8::Object> msg) {

		Local<v8::String> tagsKey = Nan::New<v8::String>("tags").ToLocalChecked();

		if(msg->Has(Nan::GetCurrentContext(), tagsKey).ToChecked()) {

			Local<v8::Object> tags = Local<v8::Object>::Cast(msg->Get(Nan::GetCurrentContext(), tagsKey).ToLocalChecked());

			Local<v8::Array> msgTags = tags->GetPropertyNames(Nan::GetCurrentContext()).ToLocalChecked();

			for(int i=0; i < (int)msgTags->Length(); i++) {
			    String::Utf8Value value(Isolate::GetCurrent(), Nan::To<v8::String>(tags->Get(Nan::GetCurrentContext(), msgTags->Get(Nan::GetCurrentContext(), i).ToLocalChecked()).ToLocalChecked()).ToLocalChecked());

				map->setField(
						Nan::To<int32_t>(msgTags->Get(Nan::GetCurrentContext(), i).ToLocalChecked()).ToChecked(),
						std::string(*value)
				);
			}

		}

	}

	static void addFixGroups(FIX::FieldMap* map, Local<v8::Object> msg) {
		Local<v8::String> groupKey = Nan::New<v8::String>("groups").ToLocalChecked();

		// TODO: add type checking and dev-helpful error throwing


		if(msg->Has(Nan::GetCurrentContext(), groupKey).ToChecked()) {


			Local<v8::Array> groups = Local<v8::Array>::Cast(msg->Get(Nan::GetCurrentContext(), groupKey).ToLocalChecked());

			for(int i = 0; i < (int) groups->Length(); i++) {


				Local<v8::Object> groupObj = groups->Get(Nan::GetCurrentContext(), i).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked();

				Local<v8::String> delimKey = Nan::New<v8::String>("delim").ToLocalChecked();
				Local<v8::String> indexKey = Nan::New<v8::String>("index").ToLocalChecked();

				if( ! groupObj->Has(Nan::GetCurrentContext(), indexKey).ToChecked()) {
						Nan::ThrowError("no index property found on object");
				}

				if( ! groupObj->Has(Nan::GetCurrentContext(), delimKey).ToChecked()) {
						Nan::ThrowError("no delim property found on object");
				}


				Local<v8::String> entriesKey = Nan::New<v8::String>("entries").ToLocalChecked();

				if( ! groupObj->Has(Nan::GetCurrentContext(), entriesKey).ToChecked()) {
						Nan::ThrowError("no entries property found on object");
				}

				Local<v8::Array> groupEntries = Local<v8::Array>::Cast(groupObj->Get(Nan::GetCurrentContext(), entriesKey).ToLocalChecked());

				for (int j = 0; j < (int) groupEntries->Length(); j++) {

					Local<v8::Object> entry = groupEntries->Get(Nan::GetCurrentContext(), j).ToLocalChecked()->ToObject(Nan::GetCurrentContext()).ToLocalChecked();

					Local<v8::String> tagKey = Nan::New<v8::String>("tags").ToLocalChecked();

					if(entry->Has(Nan::GetCurrentContext(), groupKey).ToChecked() || entry->Has(Nan::GetCurrentContext(), tagKey).ToChecked()) {

						FIX::Group* group = new FIX::Group(
                            Nan::To<int>(groupObj->Get(Nan::GetCurrentContext(), indexKey).ToLocalChecked()).ToChecked(),
                            Nan::To<int>(groupObj->Get(Nan::GetCurrentContext(), delimKey).ToLocalChecked()).ToChecked());

						addFixTags(group, entry);
						addFixGroups(group, entry);

						map->addGroup(group->field(), *group);

						delete group;

					} else {

						FIX::Group* group = new FIX::Group(
                                Nan::To<int>(groupObj->Get(Nan::GetCurrentContext(), indexKey).ToLocalChecked()).ToChecked(),
                                Nan::To<int>(groupObj->Get(Nan::GetCurrentContext(), delimKey).ToLocalChecked()).ToChecked());

						// compat for old, non-nested format

						Local<v8::Array> entryTags = entry->GetPropertyNames(Nan::GetCurrentContext()).ToLocalChecked();

						for(int k=0; k < (int) entryTags->Length(); k++) {

							Local<v8::String> prop = entryTags->Get(Nan::GetCurrentContext(), k).ToLocalChecked()->ToString(Nan::GetCurrentContext()).ToLocalChecked();
                            String::Utf8Value keyStr(Isolate::GetCurrent(), Nan::To<v8::String>(prop).ToLocalChecked());
                            String::Utf8Value valueStr(Isolate::GetCurrent(), Nan::To<v8::String>(entry->Get(Nan::GetCurrentContext(), prop).ToLocalChecked()).ToLocalChecked());

							group->setField(atoi(std::string(*keyStr).c_str()), std::string(*valueStr));

						}

						map->addGroup(group->field(), *group);

						delete group;

					}
				}
			}
		}
	}

	static void addFixTrailer(FIX::Message* message, Local<v8::Object> msg) {

		FIX::Trailer &msgTrailer = message->getTrailer();

		Local<v8::String> trailerKey = Nan::New<v8::String>("trailer").ToLocalChecked();

		if(msg->Has(Nan::GetCurrentContext(), trailerKey).ToChecked()) {

			Local<v8::Object> trailer = Local<v8::Object>::Cast(msg->Get(Nan::GetCurrentContext(), trailerKey).ToLocalChecked());
			Local<v8::Array> trailerTags = trailer->GetPropertyNames(Nan::GetCurrentContext()).ToLocalChecked();

			for(int i=0; i<(int)trailerTags->Length(); i++) {

				Local<v8::String> prop = trailerTags->Get(Nan::GetCurrentContext(), i).ToLocalChecked()->ToString(Nan::GetCurrentContext()).ToLocalChecked();
                String::Utf8Value keyStr(Isolate::GetCurrent(), Nan::To<v8::String>(prop).ToLocalChecked());
                String::Utf8Value valueStr(Isolate::GetCurrent(), Nan::To<v8::String>(trailer->Get(Nan::GetCurrentContext(), prop).ToLocalChecked()).ToLocalChecked());

				msgTrailer.setField(atoi(std::string(*keyStr).c_str()), std::string(*valueStr));

			}

		}

	}

	static void js2Fix(FIX::Message* message, Local<v8::Object> msg) {

		addFixHeader(message, msg);

		addFixTags(message, msg);

		addFixGroups(message, msg);

		addFixTrailer(message, msg);

	}

	static void addJsHeader(Local<v8::Object> msg, const FIX::Message* message) {
		Local<v8::Object> header = Nan::New<v8::Object>();
		FIX::Header messageHeader = message->getHeader();

		for(FIX::FieldMap::const_iterator it = messageHeader.begin(); it != messageHeader.end(); ++it)
		{
			header->Set(Nan::New<Integer>(it->getTag()), Nan::New<v8::String>(it->getString().c_str()).ToLocalChecked());
		}

		msg->Set(Nan::GetCurrentContext(), Nan::New<v8::String>("header").ToLocalChecked(), header);
	}

	static void addJsTags(Local<v8::Object> msg, const FIX::FieldMap* map) {
		Local<v8::Object> tags = Nan::New<v8::Object>();
		int noTags = 0;

		for(FIX::FieldMap::const_iterator it = map->begin(); it != map->end(); ++it)
		{
			tags->Set(Nan::New<Integer>(it->getTag()), Nan::New<v8::String>(it->getString().c_str()).ToLocalChecked());
			noTags++;
		}

		if (noTags > 0) {
			msg->Set(Nan::GetCurrentContext(), Nan::New<v8::String>("tags").ToLocalChecked(), tags);
		}
	}

	static void addJsTrailer(Local<v8::Object> msg, const FIX::Message* message) {
		Local<v8::Object> trailer = Nan::New<v8::Object>();
		FIX::Trailer messageTrailer = message->getTrailer();

		for(FIX::FieldMap::const_iterator it = messageTrailer.begin(); it != messageTrailer.end(); ++it)
		{
			trailer->Set(Nan::New<Integer>(it->getTag()), Nan::New<v8::String>(it->getString().c_str()).ToLocalChecked());
		}

		msg->Set(Nan::GetCurrentContext(), Nan::New<v8::String>("trailer").ToLocalChecked(), trailer);
	}

	static void addJsGroups(Local<v8::Object> msg, const FIX::FieldMap* map) {
		Local<v8::Object> groups = Nan::New<v8::Object>();
		int noGroups = 0;

		for(FIX::FieldMap::g_const_iterator it = map->g_begin(); it != map->g_end(); ++it) {
			std::vector< FIX::FieldMap* > groupVector = it->second;
			Local<v8::Array> groupList = Nan::New<v8::Array>(groupVector.size());
			int i = 0;

			for(std::vector< FIX::FieldMap* >::iterator v_it = groupVector.begin(); v_it != groupVector.end(); ++v_it) {
				Local<v8::Object> groupEntry = Nan::New<v8::Object>();

				FIX::FieldMap* fields = *v_it;

				addJsTags(groupEntry, fields);

				addJsGroups(groupEntry, fields);

				groupList->Set(Nan::GetCurrentContext(), i, groupEntry);

				i++;
			}

			groups->Set(Nan::GetCurrentContext(), Nan::New<Integer>(it->first), groupList);

			noGroups++;
		}

		if (noGroups > 0) {
			msg->Set(Nan::GetCurrentContext(), Nan::New<v8::String>("groups").ToLocalChecked(), groups);
		}
	}

	static void fix2Js(Local<v8::Object> msg, const FIX::Message* message) {

		addJsHeader(msg, message);

		addJsTags(msg, message);

		addJsTrailer(msg, message);

		addJsGroups(msg, message);

	}

	static Local<Value> sessionIdToJs(const FIX::SessionID* sessionId) {
		Local<v8::Object> session = Nan::New<v8::Object>();

		session->Set(Nan::GetCurrentContext(), Nan::New<v8::String>("beginString").ToLocalChecked(), Nan::New<v8::String>(sessionId->getBeginString().getString().c_str()).ToLocalChecked());
		session->Set(Nan::GetCurrentContext(), Nan::New<v8::String>("senderCompID").ToLocalChecked(), Nan::New<v8::String>(sessionId->getSenderCompID().getString().c_str()).ToLocalChecked());
		session->Set(Nan::GetCurrentContext(), Nan::New<v8::String>("targetCompID").ToLocalChecked(), Nan::New<v8::String>(sessionId->getTargetCompID().getString().c_str()).ToLocalChecked());
		session->Set(Nan::GetCurrentContext(), Nan::New<v8::String>("sessionQualifier").ToLocalChecked(), Nan::New<v8::String>(sessionId->getSessionQualifier().c_str()).ToLocalChecked());

		return session;
	}

	static FIX::SessionID jsToSessionId(Local<v8::Object> sessionId) {
        String::Utf8Value beginString(Isolate::GetCurrent(), Nan::To<v8::String>(sessionId->Get(Nan::GetCurrentContext(), Nan::New<v8::String>("beginString").ToLocalChecked()).ToLocalChecked()).ToLocalChecked());
        String::Utf8Value senderCompId(Isolate::GetCurrent(), Nan::To<v8::String>(sessionId->Get(Nan::GetCurrentContext(), Nan::New<v8::String>("senderCompID").ToLocalChecked()).ToLocalChecked()).ToLocalChecked());
        String::Utf8Value targetCompId(Isolate::GetCurrent(), Nan::To<v8::String>(sessionId->Get(Nan::GetCurrentContext(), Nan::New<v8::String>("targetCompID").ToLocalChecked()).ToLocalChecked()).ToLocalChecked());
		return FIX::SessionID(std::string(*beginString),
				std::string(*senderCompId),
				std::string(*targetCompId));
	}
};

#endif /* FIXMESSAGEUTIL_H_ */
