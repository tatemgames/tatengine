/*
 *  teHTTPNetwork.h
 *  TatEngine
 *
 *  Created by Dmitry Ivanov, Vadim Luchko on 05/10/13.
 *  Copyright 2013 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_HTTPNETWORK_2_H
#define TE_HTTPNETWORK_2_H

#include "teTypes.h"
#include "teString.h"

namespace te
{
	namespace net
	{
		const u32 teHTTPDefaultBufferSize = 128 * 1024;
		const u32 teHTTPSocketReadBufferSize = 128 * 1024;
		const u32 teHTTPSleepTime = 20;
		const u32 teHTTPMaxAttempts = 10;

		struct teHTTPUrl;
		struct teHTTPRequest;
		struct teHTTPResponse;
		class teHTTPSocket;
		class teHTTPNetwork;
		teHTTPNetwork * GetHTTPNetwork();

		struct teHTTPUrl
		{
			c8 buffer[2 * 1024];
			teString host;
			u16 port;
			teString path;
			teString query;

			teHTTPUrl() {}
			teHTTPUrl(const teString & setURI) {SetURI(setURI);}
			teHTTPUrl(const c8 * setURI) {SetURI(setURI);}
			teHTTPUrl(const teHTTPUrl & other) {CopyFrom(other);}

			teHTTPUrl & operator = (const teHTTPUrl & other)
			{
				return CopyFrom(other);
			}

			void Clear();
			teHTTPUrl & CopyFrom(const teHTTPUrl & other);

			teHTTPUrl & SetURI(const teString & setURI);
			teHTTPUrl & SetHTTP(const teString & setHost, u16 setPort = 80);
			teHTTPUrl & SetPort(u16 setPort);
			teHTTPUrl & SetPath(const teString & setPath);
			teHTTPUrl & SetQuery(const teString & setQuery);
			teHTTPUrl & SetHandleData(); // copy string to internal buffer
		};

		enum EHTTPCallBackResult
		{
			HCBR_OK = 0, // result is accepted
			HCBR_FORCE_CLOSE_CONNECTION, // if you get something awful with willBeMore = true, and you dont want to accept it
			HCBR_TRY_SEND_AGAIN, // stop working on current status, close and reopen connection, increment resendCount on request
		};

		// dataSize == -1 if error
		typedef EHTTPCallBackResult (*teHTTPNetCallback)(const teHTTPRequest & request, const c8 * data, s32 dataSize, u1 willBeMore);

		struct teHTTPSocket
		{
			s32 socketId;
			u1 connected;

			teHTTPSocket();
			~teHTTPSocket();

			u1 Connect(const teHTTPUrl & url);
			void Disconnect();
			void SendEnded();

			u1 Write(const void * data, u32 dataSize);
			u1 Write(teString data) {return Write(data.GetRawRO(), data.GetSize());}
			s32 Read(void * buffer, u32 bufferSize);
		};

		struct teHTTPRequest
		{
			enum ERequestType
			{
				RT_GET = 0,
				RT_DELETE,
				RT_POST,
				RT_PUT
			};

			enum EWorkMode
			{
				WM_KEEP_ALIVE = 0,
				WM_ONCE,
				WM_TRYING,
			};

			enum EErrorType
			{
				ET_NO_ERROR = 0,
				ET_CONNECT_FAIL,
				ET_USER_HEADERS_TOO_BIG,
				ET_SEND_FAIL,
				ET_OPEN_FILE_FAIL,
				ET_HTTP_HEADER_ERROR,
				ET_HTTP_TIMEOUT,
				ET_UNKNOWN,
			};

			teHTTPUrl url;
			ERequestType type;
			const void * postData;
			u32 postDataSize;
			teString headers;
			teHTTPNetCallback callback;
			teptr_t userData;
			c8 fileName[1024];
			teHTTPSocket socket;
			core::IBuffer * fileBuffer;
			c8 * readBuffer;
			u32 readBufferSize;
			u32 readBufferReadedSize;
			EWorkMode mode;
			EErrorType error; // contains error code if something failed
			u8 errorsCount;
			u8 resendCount;
			u16 httpCode;
			u32 contentLength;
			u32 chunkSize;
			u1 chunkMode;
			u1 sended;
			u1 clear;
			u1 readedHeader;
			u1 redirected;
			u1 resended;
			c8 internalPostBuffer[2 * 1024];

			teHTTPRequest();
			teHTTPRequest(const teString & setURI);
			teHTTPRequest(const c8 * setURI);
			teHTTPRequest(const teHTTPUrl & setURL);
			teHTTPRequest(const teHTTPRequest & other);
			~teHTTPRequest();

			teHTTPRequest & operator = (const teHTTPRequest & other)
			{
				return CopyFrom(other);
			}

			teHTTPRequest & CopyFrom(const teHTTPRequest & other);
			void Clear();
			void PrepareToResend();
			u1 OpenFile();
			u1 FinalizeHeaders(c8 * output, u32 outputSize, u32 * resultSize = NULL);
			EHTTPCallBackResult Write(const void * data, u32 size);
			EHTTPCallBackResult OnOk();
			EHTTPCallBackResult OnError();

			u1 IsHTTPCodeOk() const {return httpCode == 200;}

			u1 IsPostHandledInternal() {return (postData >= internalPostBuffer) && (postData < internalPostBuffer + sizeof(internalPostBuffer));}

			// data from SetPost and SetPut will be copied to internal post buffer if dataSize <= sizeof(internalPostBuffer)
			// if not - you must guarantee that pointer will be valid between Add(request) and Callback calls
			// if IsPostHandledInternal returns true - post data handled by teHTTPRequest itself

			teHTTPRequest & SetURL(const teHTTPUrl & setURL); // URL by default is get
			teHTTPRequest & SetDelete();
			teHTTPRequest & SetPost(const void * data, u32 dataSize);
			teHTTPRequest & SetPost(teString data);
			teHTTPRequest & SetPut(const void * data, u32 dataSize);
			teHTTPRequest & SetPut(teString data);
			teHTTPRequest & SetHeaders(teString setHeaders);
			teHTTPRequest & SetUserData(teptr_t setUserData);
			teHTTPRequest & SetSaveToFile(teString setFileName);
			teHTTPRequest & SetReadBuffer(c8 * buffer, u32 bufferSize);
			teHTTPRequest & SetMode(EWorkMode setMode);
			teHTTPRequest & SetCallBack(teHTTPNetCallback setCallback);
		};

		class teHTTPNetwork
		{
		public:
			teHTTPNetwork();
			~teHTTPNetwork();

			u32 Add(const teHTTPRequest & request); // return request index
			void Remove(u32 index);
			void Remove(const teHTTPRequest & request);

			teConstArray<teHTTPRequest> & Get() {return requests;}
			const teConstArray<teHTTPRequest> & Get() const {return requests;}

			void GetDefaultBuffer(c8 ** buffer, u32 & size);

		protected:
			teConstArray<teHTTPRequest> requests;
			c8 defaultBuffer[teHTTPDefaultBufferSize];

			static u1 Process(teHTTPRequest & request, c8 * buffer, u32 bufferSize);
			static u1 ProcessResult(teHTTPRequest & request, EHTTPCallBackResult result);
			static void * teHTTPThreadRoutine(void * data);
		};
	}
}

#endif
