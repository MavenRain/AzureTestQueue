#pragma once
#include <cpprest/http_client.h>
using namespace ::pplx;
using namespace std;

namespace QED
{
	class ServiceQueue
	{
	public:
		void SendJSON(const wstring&, const wstring&);
		void ReceiveJSON(const wstring&, const wstring&);
	};
}