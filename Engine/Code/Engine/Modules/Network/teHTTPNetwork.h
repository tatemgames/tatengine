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
			teString host;
			u16 port;
			teString path;
			teString query;

			teHTTPUrl() {}
			teHTTPUrl(const teString & setURI) {SetHTTP(setURI);}
			teHTTPUrl(const c8 * setURI) {SetHTTP(setURI);}

			teHTTPUrl & SetHTTP(const teString & setHost);
			teHTTPUrl & SetPort(u16 setPort);
			teHTTPUrl & SetPath(const teString & setPath);
			teHTTPUrl & SetQuery(const teString & setQuery);
		};

		// dataSize == -1 if error
		typedef void (*teHTTPNetCallback)(const teHTTPRequest & request, const c8 * data, s32 dataSize, u1 willBeMore);

		struct teHTTPSocket
		{
			s32 socketId;
			u1 connected;

			teHTTPSocket();
			~teHTTPSocket();

			u1 Connect(const teHTTPUrl & url);
			void Disconnect();

			u1 Write(const void * data, u32 dataSize);
			u1 Write(teString data) {return Write(data.GetRawRO(), data.GetSize());}
			s32 Read(void * buffer, u32 bufferSize);
		};

		struct teHTTPRequest
		{
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
				ET_HTTP_CODE_ERROR,
				ET_UNKNOWN,
			};

			teHTTPUrl url;
			const void * postData;
			u32 postDataSize;
			teString headers;
			teHTTPNetCallback callback;
			teptr_t userData;
			teString fileName;
			teHTTPSocket socket;
			core::IBuffer * fileBuffer;
			c8 * readBuffer;
			u32 readBufferSize;
			u32 readBufferReadedSize;
			EWorkMode mode;
			EErrorType error; // contains error code if something failed
			u8 errorsCount;
			u32 chunkSize;
			u1 chunkMode;
			u1 sended;
			u1 clear;
			u1 readedHeader;

			teHTTPRequest();
			teHTTPRequest(const teHTTPUrl & setURL);
			~teHTTPRequest();

			void Clear();
			u1 OpenFile();
			u1 FinalizeHeaders(c8 * output, u32 outputSize);
			void Write(const void * data, u32 size);
			void OnOk();
			void OnError();

			teHTTPRequest & SetURL(const teHTTPUrl & setURL);
			teHTTPRequest & SetPost(const void * data, u32 dataSize);
			teHTTPRequest & SetPost(teString data);
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
			static void * teHTTPThreadRoutine(void * data);
		};
	}
}

#endif
