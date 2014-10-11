#include "ServiceQueue.h"

using namespace QED;

void wmain()
{
	ServiceQueue* queue = new ServiceQueue();
	queue->SendJSON(L"https://solomonrain.servicebus.windows.net/solomonrainq/messages", L"SharedAccessSignature sr=https%3A%2F%2Fsolomonrain.servicebus.windows.net%2Fsolomonrainq%2Fmessages&sig=TVnT%2FQ17hPT340jIu61Yj28XqNNo8uoRrUgVtufUscA%3D&se=1413070578&skn=solomonrain");
	queue->ReceiveJSON(L"https://solomonrain.servicebus.windows.net/solomonrainq/messages/head", L"SharedAccessSignature sr=https%3A%2F%2Fsolomonrain.servicebus.windows.net%2Fsolomonrainq%2Fmessages%2Fhead&sig=Rp0Oci7sYoEEfwlp4KQCHR%2B3PN%2BYe6oPx6lf8yc5whE%3D&se=1413070689&skn=solomonrain");
	delete queue;
	system("pause");
}