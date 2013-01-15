/*
 *  teHTTPNetwork.h
 *  TatEngine
 *
 *  Created by Vadim Luchko on 07/04/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_HTTPNETWORK_H
#define TE_HTTPNETWORK_H

#include "teTypes.h"
#include "teString.h"

namespace te
{
	namespace net
	{
		#define TE_NET_WAIT_RESPONSE_ETERNALY 0
		#define TE_NET_WAIT_RESPONSE_TIMEOUT 1
		#define TE_NET_WAIT_RESPONSE_ONCE 2
		
		const u32 teNetRespFailCount = 10; // for TE_NET_WAIT_RESPONSE_TIMEOUT
		
		const u32 teNetRespSize = 40 * 1024; 
		const u32 teNetReqSize = 256;
		const u32 teNetReqDataSize = 512;

		const u32 teNetQueueSize = 4;
		
		const u32 teNetSendBufferSize = 40 * 1024;
		const u32 teNetRecvBufferSize = 40 * 1024;
		
		const u32 teNetSleepTime = 20;
		
		const u8 teNetCommandsCount = 4;
		
		const teString defaultPage = "/";
		
		const c8 tePartSeparator[] = "THIS_IS_PART_SEPARATOR";
		
		enum EErrorNum
		{
			E_NO_ERRORS = 32000,
			E_INVALID_HOST,
			E_TIMEOUT,
			E_UNKNOWN
		};
		
		typedef void (*HTTPNetCallback)(const u16 & errorNum, void * data, const u32 & dataSize, teptr_t userData);  
		
		struct teNetCommand
		{
			c8 var[128];
			c8 val[128];
		};
		
		struct teResponse
		{
			c8 data[teNetRespSize];
			u32 dataSize;
			u32 packetID;
			u32 error;
			HTTPNetCallback callback;
			teptr_t userData;
						
			u32 nextIndex;
		};
		
		struct teRequest
		{
			c8 host[teNetReqSize];
			c8 page[teNetReqSize];
			u32 port;
			
			c8 data[teNetReqDataSize];
			u32 dataSize;
			c8 filename[teNetReqSize];
			HTTPNetCallback callback;
			teptr_t userData;
			char waitRespType;
			u32 failResCount;
			u32 error;	//-- from EErrorNum
			
			teNetCommand commands[teNetCommandsCount];
			int commandsCount;
						
			u1 sended;
			u32 packetID;
			
			u32 nextIndex;
		};
				
		class teHTTPNetwork
		{
		public:
			teHTTPNetwork();
			~teHTTPNetwork();
			
			void Write(const teString & host, u32 port, void * data = NULL, u32 dataSize = 0, HTTPNetCallback callback = NULL, teptr_t userData = 0, const teString & page = defaultPage, c8 * filename = NULL, teNetCommand * netCommands = NULL, u32 netCommandsCount = 0,  c8 waitRespType = TE_NET_WAIT_RESPONSE_ONCE);
			void * Read(u32 & dataSize);
			
			u1 IsRequestQueueComplete();
			
			friend void * ThreadRoutine(void* data);
			
			void Test(HTTPNetCallback callback);
			void PrintTest()
			{

			}
			
		protected:
			u1 Connect();
			void Disconnect();
			u1 SetAddr(const teString & addr, u32 port);
			
			u1 OpenSocket();
			
			void Analyze(u32 recvSize);
			void FormMessage(teRequest & req, c8 * buffer, u32 & bufferSize);
			
			//------- inner Info
			s32 socketID;

			u1 isConnected;
			u32 curPacketID;
			
			teConstArray<teRequest> requests;  // req queue
			u32 reqFirst, reqLast, reqCount;
			
			teConstArray<teResponse> responses;  // resp queue
			u32 respFirst, respLast, respCount;
			
			void ClearRequests();
			void ClearResponses();
			void AddToRequests(const c8 * host, const c8 * page, u32 port, c8 * data, u32 dataSize, HTTPNetCallback callback, teptr_t userData, c8 * filename, teNetCommand * netCommands, u32 netCommandsCount, c8 waitRespType);
			void AddToResponses(c8 * data, u32 dataSize, u32 errorID, u32 packetID, HTTPNetCallback callback = NULL, teptr_t userData = 0);
			void DelFromRequests();
			void DelFromResponses();
		
			c8 sendBuffer[teNetSendBufferSize];
			u32 sendBufferUsed;
			
			c8 recvBuffer[teNetRecvBufferSize];
			u32 recvBufferUsed;
		};
		
		teHTTPNetwork * GetHTTPNetwork();
		void * ThreadRoutine(void* data);
	}
	
}





#endif
