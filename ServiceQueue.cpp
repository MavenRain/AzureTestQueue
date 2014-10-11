#pragma once
#include <cpprest/http_client.h>
#include <cpprest/json.h>
#include "ServiceQueue.h"

using namespace ::pplx;
using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace QED;
using namespace utility;
using namespace std;
using namespace Concurrency::streams;

void ServiceQueue::SendJSON(const wstring& endpoint, const wstring& authcode)
{
	json::value obj;
	obj[L"key1"] = json::value::boolean(false);
	obj[L"key2"] = json::value::number(44);
	obj[L"key3"] = json::value::number(43.6);
	obj[L"key4"] = json::value::string(U("str"));
	http_client client(endpoint);
	http_request request(methods::POST);
	request.headers().add(L"Authorization", authcode);
	request.headers().add(L"Content-Type", L"application/atom+xml;type=entry;charset=utf-8");
	request.set_body(obj);
	client.request(request).then([](http_response response)
	{
		wcout << response.status_code() << "\n" << endl;
	});
}

void ServiceQueue::ReceiveJSON(const wstring& endpoint, const wstring& authcode)
{
	http_client client(endpoint);
	http_request request(methods::POST);
	request.headers().add(L"Authorization", authcode);
	client.request(request)
		.then([](http_response response)-> Concurrency::streams::istream
	{
		return response.body();
	})
		.then([](Concurrency::streams::istream result)
	{
		try
		{
			container_buffer<string> inBuffer;
			return result.read_to_end(inBuffer)
				.then([inBuffer](size_t byteRead)
			{
				const string& text = inBuffer.collection();
				cout << text << "\n" << endl;
			});
		}
		catch (const http_exception& e)
		{
			wostringstream ss;
			ss << e.what() << endl;
			wcout << ss.str();
		}
	});
}
	
