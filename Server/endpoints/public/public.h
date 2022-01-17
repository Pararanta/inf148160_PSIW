/*
* Public endpoints.
* Endpoints put out for handling request from non-established clients.
* We assume that all functions implemented here are called through clients queue by Client application.
* It awaits requests from public queue and handles them by calling service, then responding if nescessary.
*/

#ifndef PUBLIC_ENDPOINT_HEADER
#define PUBLIC_ENDPOINT_HEADER

	#include "../../types/types.h"
	#include "sys/types.h"
	#include "sys/ipc.h"

	#define PUBLIC_REQUEST_NEW_CLIENT 1001
	#define PUBLIC_RESPONSE_NEW_CLIENT_OK 11001
	#define PUBLIC_RESPONSE_NEW_CLIENT_ERROR 11001

	int start_public(int key, int service_key, int service_key_response);

#endif