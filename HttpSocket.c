/********************************************************************
 * COPYRIGHT --  
 ********************************************************************
 * Library: HTTPComm
 * File: HttpSocket.c
 * Author: sClaiborne
 * Created: April 12, 2021
 ********************************************************************
 * Implementation of library HTTPComm
 ********************************************************************/

#include <bur/plctypes.h>
#ifdef __cplusplus
	extern "C"
	{
#endif

#include "HTTPComm.h"

#ifdef __cplusplus
	};
#endif

#include "HttpUtility.h"
#include <string.h>

void httpShiftReceivePointer(HttpClient_typ* t, unsigned long bytes) {
	t->internal.tcpStream.IN.PAR.pReceiveData += bytes;
	t->internal.tcpStream.IN.PAR.MaxReceiveLength -= bytes; // TODO: Dont allow rollover
}
void httpResetReceivePointer(HttpClient_typ* t) {
	t->internal.tcpStream.IN.PAR.pReceiveData = &t->internal.rawrecvData[0];
	t->internal.bufferSize = sizeof(t->internal.rawrecvData); // this is only here becuase rawrecvData is not dynamic alloc yet
	t->internal.tcpStream.IN.PAR.MaxReceiveLength = t->internal.bufferSize;
}

void HttpClientSetError(HttpClient_typ* t, HttpErr_enum errorId) {
#ifdef debug_checks
	if(!t) return 0;
#endif
	
	t->error = 1;
	t->errorId = errorId;
}

void HttpClient(HttpClient_typ* t) {
	
	HttpCallback pCallback;
	char temp[25] = {};
	
	t->internal.bufferSize = sizeof(t->internal.tempBuffer);
	
	if(t->enable) {
		strcpy(t->internal.tcpMgr.IN.CFG.LocalIPAddress, t->localIPAddress);
		t->internal.tcpMgr.IN.CFG.LocalPort = t->localPort;
		strcpy(t->internal.tcpMgr.IN.CFG.RemoteIPAddress, t->hostname);
		t->internal.tcpMgr.IN.CFG.RemotePort = t->port ? t->port : t->https ? HTTP_HTTPS_PORT : HTTP_HTTP_PORT;;
		
		// TODO: This needs to be more dynamic
		t->internal.tcpMgr.IN.CFG.SendBufferSize = t->internal.bufferSize;
		t->internal.tcpMgr.IN.CFG.Mode = TCPCOMM_MODE_CLIENT;
//		t->internal.tcpMgr.IN.CMD.Enable = t->internal.tcpStream.Internal.CommState == TCPCOMM_ST_CLOSED;
	}
	else {
		// TODO: Stop being enable here	
		t->internal.tcpMgr.IN.CMD.Enable = 0;
		t->internal.connected = 0;
		if(t->internal.tcpStream.OUT.Active) t->internal.tcpStream.IN.CMD.Close = 1;
	}
	
	TCPManageConnection(&t->internal.tcpMgr);
	t->internal.tcpMgr.IN.CMD.Enable = t->enable;
	t->internal.tcpMgr.IN.CMD.AcknowledgeConnection = 0;
	
	if(t->internal.tcpMgr.OUT.NewConnectionAvailable) {
		t->internal.tcpMgr.IN.CMD.AcknowledgeConnection = 1;
		
		// New connection
		t->internal.tcpStream.IN.PAR.pReceiveData = &t->internal.rawrecvData;
		t->internal.tcpStream.IN.PAR.MaxReceiveLength = sizeof(t->internal.rawrecvData);
		t->internal.tcpStream.IN.PAR.pSendData = &t->internal.rawSendData;
		t->internal.tcpStream.IN.PAR.AllowContinuousReceive = 1;
		t->internal.tcpStream.IN.PAR.AllowContinuousSend = 1;
		memcpy(&t->internal.tcpStream.IN.PAR.Connection, &t->internal.tcpMgr.OUT.Connection, sizeof(t->internal.tcpMgr.OUT.Connection));
		t->internal.connected = 1;
	}
	
	TCPStreamReceive(&t->internal.tcpStream);
	
	switch (t->internal.state) {
		case HTTP_ST_IDLE:
			
			if(t->internal.api.requestBuffer.NumberValues > 0) {
				if(t->internal.tcpStream.OUT.Active) {
					BufferCopyItems(&t->internal.api.requestBuffer, 0, 1, &t->internal.currentRequest, &t->internal.bufferStatus);
					BufferRemoveTop(&t->internal.api.requestBuffer);
		
					t->internal.retries = 0;
				
					t->internal.state = HTTP_ST_HEADER;
				}
				else {
					break;
				}
			}
			else {
				// We save a cycle by only breaking if not changing states
				break;
			}

		case HTTP_ST_HEADER:
			
			// Build Header
			/////////////////
		
			// Add block
			getMethodString(t->internal.currentRequest.method, t->internal.rawSendData, sizeof(t->internal.rawSendData));
			strcat(t->internal.rawSendData, " ");
			if(t->internal.currentRequest.uri[0] != '/') strcat(t->internal.rawSendData, "/");
			strcat(t->internal.rawSendData, t->internal.currentRequest.uri);
			strcat(t->internal.rawSendData, " ");
			strcat(t->internal.rawSendData, "HTTP/1.1\r\n");
			
			strcat(t->internal.rawSendData, "Host: ");
			strcat(t->internal.rawSendData, t->internal.tcpStream.OUT.Connection.IPAddress);
			strcat(t->internal.rawSendData, ":");
			brsitoa(t->internal.tcpStream.OUT.Connection.Port, temp);
			strcat(t->internal.rawSendData, temp);
			strcat(t->internal.rawSendData, "\r\n");
			
			strcat(t->internal.rawSendData, "Keep-Alive: timeout=15, max=100\r\n");
			
			if(t->internal.currentRequest.pPayload && t->internal.currentRequest.payloadLength) {
				// TODO: Escape???
				strcat(t->internal.rawSendData, "content-length: ");
				brsitoa(t->internal.currentRequest.payloadLength, (UDINT)temp);
				strcat(t->internal.rawSendData, temp);
				strcat(t->internal.rawSendData, "\r\n");
			}
		
			UINT i = 0;
			// Add User header values 
			for (i = 0; i < sizeof(t->internal.currentRequest.userHeader)/sizeof(t->internal.currentRequest.userHeader[0]); i++) {
				// TODO: Escape???
				if(strlen(t->internal.currentRequest.userHeader[i].name) == 0) break;
				strcat((char*)t->internal.rawSendData, t->internal.currentRequest.userHeader[i].name);
				strcat((char*)t->internal.rawSendData, ": ");
				strcat((char*)t->internal.rawSendData, t->internal.currentRequest.userHeader[i].value);
				strcat((char*)t->internal.rawSendData, "\r\n");
			}
		
			// End Header
			strcat(t->internal.rawSendData, "\r\n");
			
			
			// TODO: Escape????
			// Add payload
			UDINT sendLength = strlen(t->internal.rawSendData);
			if(t->internal.currentRequest.pPayload && t->internal.currentRequest.payloadLength) {
				//sendLength += 1;
				memcpy((void*)(((UDINT)&t->internal.rawSendData) + sendLength), t->internal.currentRequest.pPayload, t->internal.currentRequest.payloadLength);
				sendLength += t->internal.currentRequest.payloadLength;
			}
			memset(((UDINT)&t->internal.rawSendData)+sendLength, 0, 1);
		
		case HTTP_ST_SEND:
			t->internal.tcpStream.IN.CMD.Send = 1;
			t->internal.tcpStream.IN.PAR.pSendData = &t->internal.rawSendData;
			t->internal.tcpStream.IN.PAR.SendLength = sendLength;
			t->internal.retries++;
			
			memset(&t->internal.parser, 0, sizeof(t->internal.parser)); // Reset this here so errors dont carry over
			
			httpResetReceivePointer(t);
			
			t->internal.state = HTTP_ST_LISTEN;
		
		case HTTP_ST_LISTEN:
			// Wait for response
			if(t->internal.tcpStream.OUT.DataReceived) {
				if (t->internal.tcpStream.OUT.ReceivedDataLength == 0) {
					t->internal.tcpStream.IN.CMD.Send = 0;
					t->internal.tcpStream.IN.CMD.Receive = 0;
					t->internal.tcpStream.IN.CMD.Close = 1;
					
					t->internal.state = HTTP_ST_ERROR;
				}
				else {
					t->internal.state = HTTP_ST_PARSE;
				
					t->internal.tcpStream.IN.CMD.AcknowledgeData = 1;
				
					t->internal.recvLength = t->internal.tcpStream.OUT.ReceivedDataLength;
				
					memset(((UDINT)&t->internal.rawrecvData)+t->internal.recvLength+1, 0, 1); // Append null char
				}
			}
			else if(t->internal.responseTimeout.Q) {
				if(t->internal.retries > HTTP_MAX_RETRIES) {
					// TODO: Maybe make this configurable
					// TODO: Then error if we do not get a response
					t->internal.state = HTTP_ST_ERROR;
				}
				else {
					// TODO: Time out, then try again a few times
					t->internal.state = HTTP_ST_SEND;
				}
			}
			
			t->internal.tcpStream.IN.CMD.Receive = 1;
			
			t->internal.responseTimeout.IN = 1;
			
			if(t->internal.state != HTTP_ST_PARSE)
				break;
		
		case HTTP_ST_PARSE:
			// TODO: Parse message into header and payload
			t->internal.parser.data = t->internal.rawrecvData;
			t->internal.parser.dataLength = t->internal.recvLength;
			HttpParse(&t->internal.parser);
			
			if(t->internal.parser.partialPacket || (t->internal.parser.partialContent && t->internal.currentRequest.method != HTTP_METHOD_HEAD)) {
				// Handle partial packet
				// Shift pointer 
				httpShiftReceivePointer(t, t->internal.recvLength);
				
				t->internal.responseTimeout.IN = 0;
				t->internal.state = HTTP_ST_LISTEN;
			}
			else if(t->internal.parser.error) {
				HttpClientSetError(t, t->internal.parser.errorId);
				t->internal.state = HTTP_ST_ERROR;
			
			}
			else{
				if(HttpHeaderContains(&t->internal.parser.header.lines, "connection", "close")) {
					t->internal.tcpStream.IN.CMD.Close = 1;
				}
			
				// TODO: Call callbacks
				if(t->internal.currentRequest.successCallback) {
					pCallback = t->internal.currentRequest.successCallback;
					pCallback(t->internal.currentRequest.self, &t->internal.api, &t->internal.parser.header, t->internal.parser.content);
				}
				
				t->internal.state = HTTP_ST_CLEAN;
			}
			
			if(t->internal.state != HTTP_ST_CLEAN)
				break;
		
		case HTTP_ST_CLEAN:
			t->internal.state = HTTP_ST_IDLE;
			break;
		
		case HTTP_ST_ERROR:
		default:
			if(t->internal.currentRequest.errorCallback) {
				pCallback = t->internal.currentRequest.errorCallback;
				pCallback(t->internal.currentRequest.self, &t->internal.api, &t->internal.parser.header, t->internal.parser.content);
			}
			t->internal.state = HTTP_ST_CLEAN;
		
			break;
	}
	
	if(t->internal.tcpStream.OUT.DataReceived && t->internal.tcpStream.OUT.ReceivedDataLength == 0) {
		t->internal.tcpStream.IN.CMD.Close = 1;
		
		if(t->internal.state != HTTP_ST_IDLE) {
			t->internal.state = HTTP_ST_ERROR;
		}
	}
	else if(t->internal.tcpStream.OUT.Error) {
		// TODO: Handle Error
		t->internal.tcpStream.IN.CMD.Close = 1;
		t->internal.tcpStream.IN.CMD.AcknowledgeError = 1;
		
		
		if(t->internal.state != HTTP_ST_IDLE) {
			t->internal.state = HTTP_ST_ERROR;
		}
	}
	
	if(t->internal.tcpStream.IN.CMD.Close) {
		t->internal.connected = 0;
		t->internal.tcpStream.IN.CMD.Send = 0;
		t->internal.tcpStream.IN.CMD.Receive = 0;
		t->internal.tcpMgr.IN.CMD.Enable = 0;
	}
	TCPStreamSend(&t->internal.tcpStream);
	t->internal.tcpStream.IN.CMD.Send = 0;
	t->internal.tcpStream.IN.CMD.Close = 0;
	t->internal.tcpStream.IN.CMD.AcknowledgeError = 0;
    
	if(t->internal.responseTimeout.Q)
		t->internal.responseTimeout.IN = 0;
	TON(&t->internal.responseTimeout);
	t->internal.responseTimeout.IN = 0;
	t->internal.responseTimeout.PT = 5000; // TODO: Make configurable
	
	t->ident = &t->internal.api;
	if(!t->internal.api.requestBuffer.Data) {
		BufferInit( &t->internal.api.requestBuffer, 25, sizeof( t->internal.currentRequest ));
	}
	
	t->connected = t->internal.connected;
}
