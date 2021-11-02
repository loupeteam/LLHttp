/********************************************************************
 * COPYRIGHT --  
 ********************************************************************
 * Library: HTTPComm
 * File: HttpServer.c
 * Author: sClaiborne
 * Created: September 17, 2021
 ********************************************************************
 * Implementation of library HTTPComm
 ********************************************************************/

#ifdef __cplusplus
	extern "C"
	{
#endif

#include "LLHttpH.h"

#ifdef __cplusplus
	};
#endif

#ifndef TMP_alloc
#define TMP_alloc(s, l) 0;*(l) = (UDINT)malloc(s)
#endif
#ifndef TMP_free
#define TMP_free(s, l) 0;free((void*)l)
#endif
#ifndef brsitoa
#define brsitoa(a,b) strlen(itoa(a,(char*)b,10))
#endif

#include "HttpUtility.h"
#include <string.h>

#define min(a,b) (((a)<(b))?(a):(b))

void HttpShiftReceivePointer(LLHttpServerInternalClient_typ* client, unsigned long bytes) {
	client->tcpStream.IN.PAR.pReceiveData += bytes;
	client->tcpStream.IN.PAR.MaxReceiveLength -= bytes; // TODO: Dont allow rollover
}
void HttpResetReceivePointer(LLHttpServerInternalClient_typ* client) {
	client->tcpStream.IN.PAR.pReceiveData = client->pReceiveData;
	client->tcpStream.IN.PAR.MaxReceiveLength = client->receiveDataSize;
}
void HttpConnect(LLHttpServerInternalClient_typ* client, TCPConnection_Desc_typ* connection) {
	if(!client || !connection) return;
	client->connected = 1;
	memcpy(&client->tcpStream.IN.PAR.Connection, connection, sizeof(client->tcpStream.IN.PAR.Connection));
	
	client->tcpStream.IN.CMD.Receive = 1;
	client->tcpStream.IN.PAR.AllowContinuousReceive = 1;
	client->tcpStream.IN.PAR.AllowContinuousSend = 1;
	
	client->tcpStream.IN.PAR.pSendData = client->pSendData;
	
	HttpResetReceivePointer(client);
	
}
void HttpDisconnect(LLHttpServerInternalClient_typ* client) {
	if(!client) return;
	client->connected = 0;
	client->tcpStream.IN.CMD.Close = 1;
	client->tcpStream.IN.CMD.Receive = 0;
}

void HttpServerSetError(LLHttpServer_typ* t, LLHttpServerInternalClient_typ* client, LLHttpErr_enum errorId) {
	#ifdef debug_checks
	if(!t || !client) return 0;
	#endif
	
	t->error = 1;
	t->errorId = errorId;
	
	client->error = 1;
	client->errorId = errorId;
}

void LLHttpServerInit(LLHttpServer_typ* t) {
	TMP_alloc(sizeof(LLHttpServerInternalClient_typ)*t->numClients, &t->internal.pClients);
	t->internal.numClients = t->numClients;
	int index;
	for (index = 0; index < t->internal.numClients; index++) {
		t->internal.pClients[index];
		BufferInit(&t->internal.pClients[index].receivedBuffer, 10, sizeof(LLHttpInternalRequest_typ)+t->bufferSize);
		BufferInit(&t->internal.pClients[index].sendBuffer, 10, sizeof(LLHttpInternalRequest_typ)+t->bufferSize);
		TMP_alloc(sizeof(LLHttpInternalRequest_typ)+t->bufferSize, &t->internal.pClients[index].pCurrentResponse);
		TMP_alloc(sizeof(LLHttpInternalRequest_typ)+t->bufferSize, &t->internal.pClients[index].pCurrentRequest);
		TMP_alloc(sizeof(LLHttpInternalRequest_typ)+t->bufferSize, &t->internal.pClients[index].pRecvRequest);
		
		TMP_alloc(sizeof(LLHttpInternalRequest_typ)+t->bufferSize, &t->internal.pClients[index].pReceiveData);
		t->internal.pClients[index].receiveDataSize = sizeof(LLHttpInternalRequest_typ)+t->bufferSize;
		
		TMP_alloc(sizeof(LLHttpInternalRequest_typ)+t->bufferSize, &t->internal.pClients[index].pSendData);
		t->internal.pClients[index].sendDataSize = sizeof(LLHttpInternalRequest_typ)+t->bufferSize;
		
		BufferInit(&t->internal.pClients[index].api.responseBuffer, 10, sizeof(LLHttpServiceResponse_typ));
	}
	
	
	//t->internal.pClients->recievedBufferSize;
	
	//t->internal.pClients->api;
	
	BufferInit(&t->internal.api.handlers, 50, sizeof(LLHttpHandler_typ));
	
	t->internal.initialized = 1;
}

void LLHttpServer(LLHttpServer_typ* t) {
	int i;
	LLHttpServerInternalClient_typ* client;
	LLHttpCallback pCallback;
	
	if(!t) return;
	
	if(!t->internal.initialized) LLHttpServerInit(t);
	
	if(t->enable) {
		strcpy(t->internal.tcpMgr.IN.CFG.LocalIPAddress, t->ipAddress);
		t->internal.tcpMgr.IN.CFG.LocalPort = t->port ? t->port : t->https ? LLHTTP_HTTPS_PORT : LLHTTP_HTTP_PORT; // Readablilty ??
		// No need to populate remote IP and port as we are a server :)
		t->internal.tcpMgr.IN.CFG.UseSSL = t->https;
		t->internal.tcpMgr.IN.CFG.SSLCertificate = t->sslIndex;
		
		// TODO: This needs to be more dynamic
		t->internal.tcpMgr.IN.CFG.SendBufferSize = t->internal.bufferSize;
		t->internal.tcpMgr.IN.CFG.Mode = TCPCOMM_MODE_SERVER;
		//t->internal.tcpMgr.IN.CMD.Enable = t->internal.tcpStream.Internal.CommState == TCPCOMM_ST_CLOSED;
	}
	else {
		// Stop being enable here	
		t->internal.tcpMgr.IN.CMD.Enable = 0;
		// TODO: Force disconnect clients, like a jerk 
		for (i = 0; i < t->internal.numClients; i++) {
			client = &t->internal.pClients[i]; // Write to local variable for easier use
			if(client->connected || client->tcpStream.OUT.Active) {
				HttpDisconnect(client);
			}
		}
	}
	
	
	TCPManageConnection(&t->internal.tcpMgr);
	t->internal.tcpMgr.IN.CMD.Enable = t->enable;
	if(t->internal.tcpMgr.OUT.NewConnectionAvailable) {
		for (i = 0; i < t->internal.numClients; i++) {
			client = &t->internal.pClients[i];
			if(!client->connected) {
				t->internal.tcpMgr.IN.CMD.AcknowledgeConnection = 1;
				HttpConnect(client, &t->internal.tcpMgr.OUT.Connection);
				break;
			}
		}
	}
	
	t->internal.numClientsConnected = 0; // Reset num connected clients so we can recount cyclically
	for (i = 0; i < t->internal.numClients; i++) {
		client = &t->internal.pClients[i]; // Write to local variable for easier use
		//if(!client->connected) continue; // We dont do anything on client not connected
		if(client->connected) t->internal.numClientsConnected++; // Each connected client adds to num connected
		
		LLHttpInternalRequest_typ data;
		
		TCPStreamReceive(&client->tcpStream);
		if(client->tcpStream.OUT.DataReceived) {
			if (client->tcpStream.OUT.ReceivedDataLength == 0) {
				HttpDisconnect(client);
			}
			else {
				client->tcpStream.IN.CMD.AcknowledgeData = 1;
				client->recvLength = client->tcpStream.OUT.ReceivedDataLength;
				memset(((UDINT)client->pReceiveData)+client->recvLength+1, 0, 1); // Append null char
				// TODO: Parse 
				client->parser.data = client->pReceiveData;
				client->parser.dataLength = client->recvLength;
				LLHttpParse(&client->parser);
				
				// TODO: Check for partial packet
				if(client->parser.partialPacket || (client->parser.partialContent)) { // TODO: We should handle expect: 100
					// TODO: Handle partial packets...
					
					// Shift pointer
					HttpShiftReceivePointer(client, client->recvLength);
				}
				else if(client->parser.error) {
					HttpServerSetError(t, client, client->parser.errorId);
				}
				else {
					HttpResetReceivePointer(client);
					
					// TODO: Set response timer
					client->requestTimer.IN = 1;
					client->requestTimer.PT = 5000;
					
					memcpy(&client->pRecvRequest->header, &client->parser.header, sizeof(data.header));
					if(client->parser.contentPresent) {
						memcpy(&client->pRecvRequest->contentStart, 
							client->parser.content, 
							min(client->parser.header.contentLength+1, t->bufferSize));
					
						((char*)&client->pRecvRequest->contentStart)[t->bufferSize-1] = '\0';
					}
					else {
						((char*)&client->pRecvRequest->contentStart)[0] = '\0';
					}
					
					// Add to Que
					BufferAddToTop(&client->receivedBuffer, client->pRecvRequest);
				
				}
			}
		}
		
		// If we can push out new command
		if(client->receivedBuffer.NumberValues > 0 && !client->requestActive) {
			BufferCopyItems(&client->receivedBuffer, 0, 1, client->pCurrentRequest, &client->bufferStatus);
			LLHttpHandler_typ* handler;
			unsigned long numMatches = 0;
			// Check if the handlers for this topic is availible 
			for (i = 0; i < t->internal.api.handlers.NumberValues; i++) {
				handler = BufferGetItemAdr(&t->internal.api.handlers, i, &client->bufferStatus);
				// TODO: Check busy
				if(LLHttpMethodMatch(handler->method, client->pCurrentRequest->header.method) && LLHttpUriMatch(handler->uri, client->pCurrentRequest->header.uri)) {
					handler->client = client;
					numMatches++;
					pCallback = handler->newMessageCallback;
					if(pCallback)
						pCallback(handler->self, &client->api, &client->pCurrentRequest->header, &client->pCurrentRequest->contentStart);
				}
			}
			if(numMatches == 0) {
				// Check if the handlers for this topic is availible 
				for (i = 0; i < t->internal.api.handlers.NumberValues; i++) {
					handler = BufferGetItemAdr(&t->internal.api.handlers, i, &client->bufferStatus);
					// TODO: Check busy
					if(handler->method == LLHTTP_METHOD_DEFAULT && LLHttpUriMatch(handler->uri, client->pCurrentRequest->header.uri)) {
						handler->client = client;
						numMatches++;
						pCallback = handler->newMessageCallback;
						if(pCallback)
							pCallback(handler->self, &client->api, &client->pCurrentRequest->header, &client->pCurrentRequest->contentStart);
					}
				}	
			}
			// Remove once used
			BufferRemoveTop(&client->receivedBuffer);
			
			client->requestActive = 1;
		}
		else {
			client->requestActive = 0;
		}
		
		// Call active request
		//		if(client->requestActive) {
		//			LLHttpHandler_typ* handler;
		//			// Check if the handlers for this topic is availible 
		//			for (i = 0; i < t->internal.handlers.NumberValues; i++) {
		//				handler = BufferGetItemAdr(&t->internal.handlers, i, status);
		//				// TODO: Check busy
		//				if(HttpMethodMatch(handler->method, data.header.method) && HttpUriMatch(handler->uri, data.header.uri)) {
		//					pCallback = handler->cyclicCallback;
		//					if(pCallback)
		//						pCallback(handler->self, &client->pCurrentRequest->header, &client->pCurrentRequest->contentStart);
		//				}
		//			}
		//		}
		
		if(client->api.responseBuffer.NumberValues > 0 && !client->tcpStream.OUT.Sending) {
			if(client->tcpStream.OUT.Active) {
				// Get Response
				BufferCopyItems(&client->api.responseBuffer, 0, 1, client->pCurrentResponse, &client->bufferStatus);
				BufferRemoveTop(&client->api.responseBuffer);
				
				// Build Packet
				if(LLHttpBuildResponse(client->pSendData, client->pCurrentResponse, client->sendDataSize, &client->tcpStream.IN.PAR.SendLength) == 0) {
					client->tcpStream.IN.CMD.Send = 1;

					client->tcpStream.IN.PAR.SendLength = strlen(client->pSendData);
					
					// Send and forget
					if(client->pCurrentResponse) {
						pCallback = client->pCurrentResponse->successCallback;
						if(pCallback) pCallback(client->pCurrentResponse->self, &client->api, 0, 0);
					}
					
				}
				else {
					if(client->pCurrentRequest) {
						pCallback = client->pCurrentResponse->errorCallback;
						if(pCallback) pCallback(client->pCurrentResponse->self, &client->api, 0, 0);
					}
				}
			}
		}
		
		// TODO: Handle recieving new sends
		TCPStreamSend(&client->tcpStream);
		client->tcpStream.IN.CMD.Send = 0;
		client->tcpStream.IN.CMD.Close = 0;
		client->tcpStream.IN.CMD.AcknowledgeError = 0;
		//client->tcpStream.IN.CMD.AcknowledgeData
		//client->tcpStream.IN.CMD.Receive
		
		//		if(client->tcpStream.OUT.DataSent) {
		//			if(client->pCurrentRequest) {
		//				pCallback = client->pCurrentResponse->successCallback;
		//				if(pCallback) pCallback(client->pCurrentResponse->self, &client->api, 0, 0);
		//			}
		//		}
		//		else if(client->tcpStream.OUT.Error) {
		//			if(client->pCurrentRequest) {
		//				pCallback = client->pCurrentResponse->errorCallback;
		//				if(pCallback) pCallback(client->pCurrentResponse->self, &client->api, 0, 0);
		//			}
		//		}
		
		
	}
	
	memcpy(&t->internal.clients, t->internal.pClients, sizeof(t->internal.clients)); // For debug
	
	// Set output
	t->ident = &t->internal.api;
	t->numConnectedClients = t->internal.numClientsConnected;
}

unsigned long generateHttpTimestamp(char* dest, unsigned long destSize, unsigned long time, unsigned long mode) {

	// Example Tue, 15 Nov 1994 12:45:26 GMT
	return stringfTime(dest, destSize, "%a, %d %b %Y %X GMT", time);
}

unsigned long appendNewLine(char* dest) {
	strcat(dest, "\r\n");
	return strlen(dest);
}

signed long LLHttpBuildResponse(unsigned long data, unsigned long _response, unsigned long dataSize, unsigned long pLen) {
	char* dest = (char*)data;
	char temp[30];
	int i;
	LLHttpServiceResponse_typ* response = _response;
	unsigned long destLen = 0;
	UtcDTGetTime_typ utcGetTime = {};
	
	// Clear dest
	dest[0] = '\0';
	
	// Status
	strcat(dest, "HTTP/");
	strcat(dest, "1.1 "); // TODO: User version here
	brsitoa(response->status, (UDINT)&temp);
	strcat(dest, temp);
	strcat(dest, " ");
	strcat(dest, HttpStatusPhrase(response->status));
	appendNewLine(dest);
	
	// Date
	utcGetTime.enable = 1;
	#ifndef _NOT_BR
	UtcDTGetTime(&utcGetTime);
	#else
	utcGetTime.DT1 = time(NULL); utcGetTime.status = 0;
	#endif
	strcat(dest, "Date: ");
	generateHttpTimestamp(temp, sizeof(temp), utcGetTime.DT1, 1); 
	strcat(dest, temp);
	appendNewLine(dest);
	
	// Server
	// TODO: What should the server be?
	strcat(dest, "Server: AR Runtime");
	appendNewLine(dest);
	
	// Last-Modified 
	// Opitional
	// TODO
	
	// Add user headers
	for (i = 0; i < sizeof(response->userHeader)/sizeof(response->userHeader[0]); i++) {
		if(response->userHeader[i].name[0] == '\0') break;
		strcat(dest, response->userHeader[i].name);
		strcat(dest, ": ");
		strcat(dest, response->userHeader[i].value);
		appendNewLine(dest);
	}
	
	
	// Content-Length
	// Optional
	if(response->payloadLength) {
		strcat(dest, "content-length: ");
		brsitoa(response->payloadLength, (UDINT)&temp);
		strcat(dest, temp);
		appendNewLine(dest);
	}
	
	
	// End Header
	appendNewLine(dest);
	
	
	// Add Payload
	// Optional
	if(response->payloadLength) {
		strcat(dest, response->pPayload);
	}
	
	if(pLen) {
		*((UDINT*)pLen) = strlen(dest);
	}
	
	return 0;
}



