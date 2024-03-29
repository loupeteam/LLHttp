/*
 * File: HttpClient.c
 * Copyright (c) 2023 Loupe
 * https://loupe.team
 * 
 * This file is part of LLHttp, licensed under the MIT License.
 */

#ifdef __cplusplus
	extern "C"
	{
#endif

#include "LLHttpH.h"

#ifdef __cplusplus
	};
#endif

#ifndef brsitoa
#define brsitoa(a,b) strlen(itoa(a,(char*)b,10))
#endif
#ifndef clock_ms
#include "time.h"
#define clock_ms() (clock()/CLOCKS_PER_SEC*1000)
#endif
#ifndef TON
	void TON(TON_typ* t)
	{
		if (t->IN)
		{
			if (t->StartTime == 0)
			{
				t->StartTime = clock_ms();
			}
			t->ET = clock_ms() - t->StartTime;
			t->Q = (t->ET >= t->PT);
		}
		else
		{
			t->ET = 0;
			t->Q = 0;
			t->StartTime = 0;
		}
	}
#endif

#include "HttpUtility.h"
#include <string.h>

BOOL httpShiftReceivePointer(LLHttpClient_typ* t, unsigned long bytes) {

	if( ( (int)t->internal.tcpStream.IN.PAR.MaxReceiveLength - bytes ) > 0 ){
		t->internal.recvLength += bytes;
		t->internal.tcpStream.IN.PAR.pReceiveData += bytes;
		t->internal.tcpStream.IN.PAR.MaxReceiveLength -= bytes;
		return 1;
	}
	else{			
		return 0;
	}

}
void httpResetReceivePointer(LLHttpClient_typ* t) {
	t->internal.tcpStream.IN.PAR.pReceiveData = &t->internal.rawrecvData[0];
	t->internal.bufferSize = sizeof(t->internal.rawrecvData) - 1; // this is only here because rawrecvData is not dynamic alloc yet
	t->internal.tcpStream.IN.PAR.MaxReceiveLength = t->internal.bufferSize;
	t->internal.recvLength = 0;
	
}

void HttpClientSetError(LLHttpClient_typ* t, LLHttpErr_enum errorId) {
#ifdef debug_checks
	if(!t) return 0;
#endif
	
	t->error = 1;
	t->errorId = errorId;
}

void LLHttpClient(LLHttpClient_typ* t) {
	
	LLHttpCallback pCallback;
	char temp[25] = {};
	
	t->internal.bufferSize = sizeof(t->internal.tempBuffer);
	
	if(t->enable) {
		strcpy(t->internal.tcpMgr.IN.CFG.LocalIPAddress, t->localIPAddress);
		t->internal.tcpMgr.IN.CFG.LocalPort = t->localPort;
		strcpy(t->internal.tcpMgr.IN.CFG.RemoteIPAddress, t->hostname);
		t->internal.tcpMgr.IN.CFG.RemotePort = t->port ? t->port : t->https ? LLHTTP_HTTPS_PORT : LLHTTP_HTTP_PORT;;
		t->internal.tcpMgr.IN.CFG.UseSSL = t->https;
		
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
		t->internal.tcpStream.IN.PAR.MaxReceiveLength = sizeof(t->internal.rawrecvData) - 1;
		t->internal.tcpStream.IN.PAR.pSendData = &t->internal.rawSendData;
		t->internal.tcpStream.IN.PAR.AllowContinuousReceive = 1;
		t->internal.tcpStream.IN.PAR.AllowContinuousSend = 1;
		memcpy(&t->internal.tcpStream.IN.PAR.Connection, &t->internal.tcpMgr.OUT.Connection, sizeof(t->internal.tcpMgr.OUT.Connection));
		t->internal.connected = 1;
	}
	
	TCPStreamReceive(&t->internal.tcpStream);

	UDINT bufSize = sizeof(t->internal.rawSendData);
	
	switch (t->internal.state) {
		case LLHTTP_ST_IDLE:
			
			if(t->internal.api.requestBuffer.NumberValues > 0) {
				if(t->internal.tcpStream.OUT.Active) {
					BufferCopyItems(&t->internal.api.requestBuffer, 0, 1, &t->internal.currentRequest, &t->internal.bufferStatus);
					BufferRemoveTop(&t->internal.api.requestBuffer);
		
					t->internal.retries = 0;
				
					t->internal.state = LLHTTP_ST_HEADER;
				}
				else {
					break;
				}
			}
			else {
				// We save a cycle by only breaking if not changing states
				break;
			}

		case LLHTTP_ST_HEADER:
			
			// Build Header
			/////////////////
			
			// Add block
			getMethodString(t->internal.currentRequest.method, (UDINT)&t->internal.rawSendData, sizeof(t->internal.rawSendData));
			strlcat(t->internal.rawSendData, " ", bufSize);
			if(t->internal.currentRequest.uri[0] != '/') strlcat(t->internal.rawSendData, "/", bufSize);
			strlcat(t->internal.rawSendData, t->internal.currentRequest.uri, bufSize);
			strlcat(t->internal.rawSendData, " ", bufSize);
			strlcat(t->internal.rawSendData, "HTTP/1.1\r\n", bufSize);
			
			strlcat(t->internal.rawSendData, "Host: ", bufSize);
			strlcat(t->internal.rawSendData, t->internal.tcpStream.OUT.Connection.IPAddress, bufSize);
			strlcat(t->internal.rawSendData, ":", bufSize);
			brsitoa(t->internal.tcpStream.OUT.Connection.Port, temp );
			strlcat(t->internal.rawSendData, temp, bufSize);
			strlcat(t->internal.rawSendData, "\r\n", bufSize);
			
			strlcat(t->internal.rawSendData, "Keep-Alive: timeout=15, max=100\r\n", bufSize);
			
			if(t->internal.currentRequest.pPayload && t->internal.currentRequest.payloadLength) {
				// TODO: Escape???
				strlcat(t->internal.rawSendData, "content-length: ", bufSize);
				brsitoa(t->internal.currentRequest.payloadLength, (UDINT)temp);
				strlcat(t->internal.rawSendData, temp, bufSize);
				strlcat(t->internal.rawSendData, "\r\n", bufSize);
			}
		
			UINT i = 0;
			// Add User header values 
			for (i = 0; i < sizeof(t->internal.currentRequest.userHeader)/sizeof(t->internal.currentRequest.userHeader[0]); i++) {
				// TODO: Escape???
				if(strlen(t->internal.currentRequest.userHeader[i].name) == 0) break;
				strlcat((char*)t->internal.rawSendData, t->internal.currentRequest.userHeader[i].name, bufSize);
				strlcat((char*)t->internal.rawSendData, ": ", bufSize);
				strlcat((char*)t->internal.rawSendData, t->internal.currentRequest.userHeader[i].value, bufSize);
				strlcat((char*)t->internal.rawSendData, "\r\n", bufSize);
			}
		
			// End Header
			strlcat(t->internal.rawSendData, "\r\n", bufSize);
			
			
			// TODO: Escape????
			// Add payload
			UDINT sendLength = strlen(t->internal.rawSendData);
			if(t->internal.currentRequest.pPayload && t->internal.currentRequest.payloadLength) {
				//sendLength += 1;
				if( sendLength + t->internal.currentRequest.payloadLength <  bufSize ){
					memcpy((void*)(((UDINT)&t->internal.rawSendData) + sendLength), t->internal.currentRequest.pPayload, t->internal.currentRequest.payloadLength);
					sendLength += t->internal.currentRequest.payloadLength;
				}
				else{
					HttpClientSetError(t, LLHTTP_ERR_PACKET_SIZE_TOO_BIG );
					t->internal.state = LLHTTP_ST_ERROR;
					break;
				}
			}
			memset(((UDINT)&t->internal.rawSendData)+sendLength, 0, 1);
		
		case LLHTTP_ST_SEND:
			t->internal.tcpStream.IN.CMD.Send = 1;
			t->internal.tcpStream.IN.PAR.pSendData = &t->internal.rawSendData;
			t->internal.tcpStream.IN.PAR.SendLength = sendLength;
			t->internal.retries++;
			
			memset(&t->internal.parser, 0, sizeof(t->internal.parser)); // Reset this here so errors dont carry over
			
			httpResetReceivePointer(t);
			
			t->internal.state = LLHTTP_ST_LISTEN;

		
		case LLHTTP_ST_LISTEN:
			// Wait for response
			if(t->internal.tcpStream.OUT.DataReceived) {
				if (t->internal.tcpStream.OUT.ReceivedDataLength == 0) {
					t->internal.tcpStream.IN.CMD.Send = 0;
					t->internal.tcpStream.IN.CMD.Receive = 0;
					t->internal.tcpStream.IN.CMD.Close = 1;
					
					t->internal.state = LLHTTP_ST_ERROR;
				}
				else {
					t->internal.state = LLHTTP_ST_PARSE;
				
					t->internal.tcpStream.IN.CMD.AcknowledgeData = 1;

					//Update the recieve buffer and length
					if( httpShiftReceivePointer(t, t->internal.tcpStream.OUT.ReceivedDataLength) == 0 ){										
						t->internal.tcpStream.IN.CMD.Send = 0;
						t->internal.tcpStream.IN.CMD.Receive = 0;
						t->internal.tcpStream.IN.CMD.Close = 1;
					
						HttpClientSetError(t, LLHTTP_ERR_PACKET_SIZE_TOO_BIG );
						t->internal.state = LLHTTP_ST_ERROR;
					}
					else{	
						memset(((UDINT)&t->internal.rawrecvData)+t->internal.recvLength, 0, 1); // Append null char
					}
				}
			}
			else if(t->internal.responseTimeout.Q) {
				if(t->internal.retries > LLHTTP_MAX_RETRIES) {
					// TODO: Maybe make this configurable
					// TODO: Then error if we do not get a response
					t->internal.state = LLHTTP_ST_ERROR;
				}
				else {
					// TODO: Time out, then try again a few times
					t->internal.state = LLHTTP_ST_SEND;
				}
			}
			
			t->internal.tcpStream.IN.CMD.Receive = 1;
			
			t->internal.responseTimeout.IN = 1;
			
			if(t->internal.state != LLHTTP_ST_PARSE)
				break;
		
		case LLHTTP_ST_PARSE:
			// TODO: Parse message into header and payload
			t->internal.parser.data = t->internal.rawrecvData;
			t->internal.parser.dataLength = t->internal.recvLength;
			LLHttpParse(&t->internal.parser);
			
			if(t->internal.parser.partialPacket || (t->internal.parser.partialContent && t->internal.currentRequest.method != LLHTTP_METHOD_HEAD)) {
				// Handle partial packet
				t->internal.responseTimeout.IN = 0;
				t->internal.state = LLHTTP_ST_LISTEN;

			}
			else if(t->internal.parser.error) {
				HttpClientSetError(t, t->internal.parser.errorId);
				t->internal.state = LLHTTP_ST_ERROR;
			
			}
			else{
				if(LLHttpHeaderContains(&t->internal.parser.header.lines, "connection", "close")) {
					t->internal.tcpStream.IN.CMD.Close = 1;
				}
			
				// TODO: Call callbacks
				if(t->internal.currentRequest.successCallback) {
					pCallback = t->internal.currentRequest.successCallback;
					pCallback(t->internal.currentRequest.self, &t->internal.api, &t->internal.parser.header, t->internal.parser.content);
				}
				
				t->internal.state = LLHTTP_ST_CLEAN;
			}
			
			if(t->internal.state != LLHTTP_ST_CLEAN)
				break;
		
		case LLHTTP_ST_CLEAN:
			t->internal.state = LLHTTP_ST_IDLE;
			break;
		
		case LLHTTP_ST_ERROR:
		default:
			if(t->internal.currentRequest.errorCallback) {
				pCallback = t->internal.currentRequest.errorCallback;
				pCallback(t->internal.currentRequest.self, &t->internal.api, &t->internal.parser.header, t->internal.parser.content);
			}
			t->internal.state = LLHTTP_ST_CLEAN;
		
			break;
	}
	
	if( t->internal.tcpStream.OUT.DataReceived && t->internal.tcpStream.OUT.ReceivedDataLength == 0 ) {
		t->internal.tcpStream.IN.CMD.Close = 1;
		
		if(t->internal.state != LLHTTP_ST_IDLE) {
			t->internal.state = LLHTTP_ST_ERROR;
		}
	}
	else if( t->abort ) {

		t->abort = 0;
		
		t->internal.tcpStream.IN.CMD.Close = 1;
		
		if(t->internal.state != LLHTTP_ST_IDLE) {
			t->internal.state = LLHTTP_ST_CLEAN;
		}
	}

	else if(t->internal.tcpStream.OUT.Error) {
		// TODO: Handle Error
		t->internal.tcpStream.IN.CMD.Close = 1;
		t->internal.tcpStream.IN.CMD.AcknowledgeError = 1;
		
		
		if(t->internal.state != LLHTTP_ST_IDLE) {
			t->internal.state = LLHTTP_ST_ERROR;
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
