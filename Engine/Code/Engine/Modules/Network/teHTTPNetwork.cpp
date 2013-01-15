/*
 *  teHTTPNetwork.cpp
 *  TatEngine
 *
 *  Created by Vadim Luchko on 07/04/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#include "TatEngineCoreConfig.h"

#ifdef TE_MODULE_NETWORK

#ifndef TE_PLATFORM_WIN
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
#else
#include <windows.h>
#include <winsock2.h>
#include <process.h>
#endif

#include "teHTTPNetwork.h"
#include "teLogManager.h"
#include "teFileManager.h"

namespace te
{
	namespace net
	{
		#ifndef TE_PLATFORM_WIN
		struct sockaddr_in netAddr;
		pthread_t threadID;
		pthread_mutex_t tempMutex;

		void teMutexLock(pthread_mutex_t * mutex)
		{
			pthread_mutex_lock(mutex);
		}

		void teMutexUnlock(pthread_mutex_t * mutex)
		{
			pthread_mutex_unlock(mutex);
		}

		void teSleep(u32 milliseconds)
		{
			usleep(milliseconds * 1000);
		}

		#else
		u32 threadID;
		HANDLE tempMutex;
		SOCKADDR_IN netAddr;

		void teMutexLock(HANDLE * mutex)
		{
			WaitForSingleObject(*mutex, INFINITE );
		}

		void teMutexUnlock(HANDLE * mutex)
		{
			ReleaseMutex(*mutex);
		}

		void teSleep(u32 milliseconds)
		{
			Sleep(milliseconds);
		}

		#endif

		teHTTPNetwork * currentHTTPNetwork = NULL;
		
		teHTTPNetwork::teHTTPNetwork()
		{
			requests.Reserve(teNetQueueSize);
			requests.Request(teNetQueueSize);
			
			responses.Reserve(teNetQueueSize);
			responses.Request(teNetQueueSize);
			
			ClearRequests();
			ClearResponses();

			isConnected = 0;
			recvBufferUsed = 0;
			memset(recvBuffer, 0, teNetRecvBufferSize);
			curPacketID = 0;
			
			#ifndef TE_PLATFORM_WIN
			pthread_mutex_init(&tempMutex, NULL);
			pthread_attr_t  attr;
			int returnVal = pthread_attr_init(&attr);
			if(returnVal)
			{
				TE_LOG("------ thread attrib fail ! ---------");
				return;
			}
			
			returnVal = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
			if(returnVal)
			{
				TE_LOG("------ thread attrib fail ! ---------");
				return;
			}
			
			int error = pthread_create(&threadID, &attr, (void *(*)(void*))ThreadRoutine, NULL);
			if(error)
			{
				TE_LOG("------ thread attrib fail ! ---------");
				return;
			}
			
			pthread_attr_destroy(&attr);
			#else
			WSADATA	wsaData;
			if( WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
			{
				TE_LOG("------ WSAStartup fail ! ---------");
				return;
			}

			tempMutex = CreateMutex( NULL, FALSE, NULL );
			_beginthread((void(_cdecl*)(void*))ThreadRoutine, 0, NULL );
			#endif
			
			currentHTTPNetwork = this;
		}
		
		teHTTPNetwork::~teHTTPNetwork()
		{
			#ifndef TE_PLATFORM_WIN
			pthread_cancel(threadID);
			pthread_mutex_destroy(&tempMutex);
			#else
			CloseHandle((HANDLE)threadID);

			if(WSACleanup() != 0)
				TE_LOG("----WSACleanup() failed: %s", WSAGetLastError());
			#endif

			currentHTTPNetwork = NULL;
		}

		u1 teHTTPNetwork::Connect()
		{
			s32 errCode = connect (socketID, (struct sockaddr*)&netAddr, sizeof(netAddr));
			if (errCode != 0)
			{
				TE_LOG("------ connect fail ! ---------");
				isConnected = 0;
				TE_LOG("errno %i", errno);
				return 0;
			}
			
			isConnected = 1;
			return 1;
		}
		
		u1 teHTTPNetwork::SetAddr(const teString & addr, u32 port)
		{
			struct hostent *host = NULL;

			#ifndef TE_PLATFORM_WIN
			bzero(&netAddr, sizeof(netAddr));
			#else
			memset(&netAddr,0, sizeof(netAddr));
			#endif
			netAddr.sin_family = AF_INET;
			netAddr.sin_port = htons(port);
			
			#ifndef TE_PLATFORM_WIN
			in_addr_t addrT = inet_addr(addr.c_str());
			if(addrT != ( in_addr_t)(-1))
				netAddr.sin_addr.s_addr = addrT;
			else
			{
				host = gethostbyname(addr.c_str());
			
				if (!host)
					return 0;
			
				memcpy(&netAddr.sin_addr, host->h_addr_list[0], host->h_length);
			}
			#else
			u32 addrT = INADDR_NONE;
			addrT = inet_addr(addr.c_str());
			if(addrT != INADDR_NONE)
				netAddr.sin_addr.s_addr = addrT;
			else
			{
				host = gethostbyname(addr.c_str());
			
				if (!host)
					return 0;
			
				memcpy(&netAddr.sin_addr, host->h_addr_list[0], host->h_length);
			}
			#endif
		
			return 1;
		}
		
		void teHTTPNetwork::Disconnect()
		{
			#ifndef TE_PLATFORM_WIN
			s32 result = shutdown(socketID, SHUT_RDWR);
			if (result < 0)
			{
				//TE_LOG("------ disconnect fail ! ---------");
				//TE_LOG("errno %i", errno);
			}
			
			result = close(socketID);
			if (result < 0)
			{
				TE_LOG("------ close fail ! ---------");
				TE_LOG("errno %i", errno);
			}
			#else
				if (closesocket(socketID) != 0)
					TE_LOG("------ close fail ! ---------");
				
			#endif
						
			isConnected = 0;
		}

		void teHTTPNetwork::Write(const teString & host, u32 port, void * data, u32 dataSize, HTTPNetCallback callback, teptr_t userData, const teString & page, c8 * filename,teNetCommand * netCommands, u32 netCommandsCount, c8 waitRespType)
		{
			u32 j;
			
			c8 * tPage;
			if(page.c_str())
				tPage = (c8*)page.c_str();
			else
				tPage = (c8*)defaultPage.c_str();
			
			if(dataSize) // POST
			{
				teMutexLock(&tempMutex);
				AddToRequests(host.c_str(), tPage, port,(c8 *)data, dataSize, callback, userData, filename, netCommands, netCommandsCount, waitRespType);
				teMutexUnlock(&tempMutex);
			}
			else //GET
			{
				teMutexLock(&tempMutex);
				AddToRequests(host.c_str(), tPage, port, NULL, 0, callback, userData, filename, netCommands, netCommandsCount, waitRespType);
				teMutexUnlock(&tempMutex);
			}
		}
		
		void * teHTTPNetwork::Read(u32 & dataSize)
		{
			if(respCount)
			{
				//if(requests[reqFirst].callback)
				//	requests[reqFirst].callback(responses[respFirst].error, responses[respFirst].data, responses[respFirst].dataSize, requests[reqFirst].userData);
				
				if(responses[respFirst].callback)
					responses[respFirst].callback(responses[respFirst].error, responses[respFirst].data, responses[respFirst].dataSize, responses[respFirst].userData);
				
				dataSize = responses[respFirst].dataSize;
				
				//DelFromRequests();
				
				void * temp = responses[respFirst].data;
				DelFromResponses();
				
				return temp;
			}
			else
			{
				u32 ttt = 0;
				dataSize = ttt;
				return NULL;
			}
			
			return NULL;//result;
		}
		
		void teHTTPNetwork::Analyze(u32 recvSize)
		{
			if(reqCount)
				AddToResponses(recvBuffer, recvSize, 0, 0, requests[reqFirst].callback, requests[reqFirst].userData);
			else
				AddToResponses(recvBuffer, recvSize, 0, 0);
		}
		
		void teHTTPNetwork::FormMessage(teRequest & req, c8 * buffer, u32 & bufferSize)
		{
            if(req.dataSize)  // POST
			{
				c8 reqHead[teNetSendBufferSize];
				
				u32 j = sprintf(reqHead, "%s", "POST ");
				j += sprintf(reqHead+j, "%s", req.page);
				j += sprintf(reqHead+j, "%s", " HTTP/1.1\r\nHost: ");
				j += sprintf(reqHead+j, "%s", req.host);
				
				if(!req.filename[0])
				{
					j += sprintf(reqHead+j, "%s", "\r\nContent-Type: application/x-www-form-urlencoded\r\nContent-Length: ");
					j += sprintf(reqHead+j, "%i", req.dataSize);
					j += sprintf(reqHead+j, "%s", "\r\n\r\n");
					bufferSize = sprintf(buffer, "%s%s\r\n", reqHead, req.data);
				}
				else
				{
					j += sprintf(reqHead+j, "%s%s%s", "\r\nContent-Type: multipart/form-data; boundary=\"",tePartSeparator,"\"\r\nContent-Length: ");
					
					//----- pre form message-----------------------
					c8 tempReqHead[teNetSendBufferSize];
					memset(tempReqHead, 0, teNetSendBufferSize);
					u32 k=0;
					
					for(u32 i = 0; i < req.commandsCount; ++i)
					{
						k += sprintf(tempReqHead + k, "%s%s%s", "--",tePartSeparator,"\r\n");
						k += sprintf(tempReqHead + k, "%s%s\r\n", "Content-Disposition: form-data; name=", req.commands[i].var);
						k += sprintf(tempReqHead + k, "\r\n%s\r\n", req.commands[i].val);
					}
					
					k += sprintf(tempReqHead + k, "%s%s%s", "--",tePartSeparator,"\r\n");
					k += sprintf(tempReqHead + k, "%s\r\n", "Content-Disposition: form-data; name=\"profile\"; filename=\"save.bin\"");
					k += sprintf(tempReqHead + k, "%s", "Content-Type: application/octet-stream\r\n");
					k += sprintf(tempReqHead + k, "%s", "Content-Transfer-Encoding: binary\r\n\r\n");
					//--------------------------------------------
					
					teString tStr = (c8*)req.filename;
					core::IBuffer * fileInput = core::GetFileManager()->OpenFile(tStr, core::CFileBuffer::FWM_READ, true);
					
					u32 tContentLength = fileInput->GetSize() + k + strlen(tePartSeparator) + 4/*2x -- */ + 2/*2x \r\n*/;
					j += sprintf(reqHead+j, "%i\r\n\r\n", tContentLength);
					j += sprintf(reqHead+j, "%s", tempReqHead);

					bufferSize = sprintf(buffer, "%s", reqHead); //-- header
					fileInput->Lock(core::BLT_READ);
					fileInput->Read(buffer + bufferSize, fileInput->GetSize()); //-- binary
					fileInput->Unlock();
					bufferSize += fileInput->GetSize();
					
					c8 rrr[] = "\r\n--";
					c8 nnn[] = "--\r\n";
					memcpy(&buffer[bufferSize], rrr, 4);
					memcpy(&buffer[bufferSize + 4], tePartSeparator, strlen(tePartSeparator));
					memcpy(&buffer[bufferSize + 4 + strlen(tePartSeparator)], nnn, 4);
					
					bufferSize += (4 + strlen(tePartSeparator) + 4);
				}
			}
			else // GET
			{
				c8 reqHead[teNetReqSize];
				
				u32 j = sprintf(reqHead, "%s", "GET ");
				j += sprintf(reqHead+j, "%s", req.page);
				j += sprintf(reqHead+j, "%s", " HTTP/1.1\r\nHost: ");
				j += sprintf(reqHead+j, "%s", req.host);
				j += sprintf(reqHead+j, "%s", "\r\n\r\n");
				
				memcpy(buffer, reqHead, strlen(reqHead));
				bufferSize = strlen(reqHead);
				
				req.packetID = curPacketID;
				
				curPacketID++;
				if(curPacketID == u32Max) curPacketID = 0; 
			}
		}
		
		void teHTTPNetwork::ClearRequests()
		{
			for(u32 i = 0; i < requests.GetAlive(); i++)
			{
				if(i < requests.GetAlive() - 1) 
					requests[i].nextIndex = i + 1;
				else
					requests[i].nextIndex = u32Max;
			}
			
			reqFirst = 0;
			reqLast = 0;
			reqCount = 0;
		}
		
		void teHTTPNetwork::ClearResponses()
		{
			for(u32 i = 0; i < responses.GetAlive(); i++)
			{
				if(i < responses.GetAlive() - 1) 
					responses[i].nextIndex = i + 1;
				else
					responses[i].nextIndex = u32Max;
			}
			
			respFirst = 0;
			respLast = 0;
			respCount = 0;
		}
		
		void teHTTPNetwork::AddToRequests(const c8 * host, const c8 * page, u32 port, c8 * data, u32 dataSize, HTTPNetCallback callback, teptr_t userData, c8 * filename, teNetCommand * netCommands, u32 netCommandsCount, c8 waitRespType)
		{
			if(reqCount >= requests.GetSize()) return;
			
			if(reqCount)
			{
				memcpy(requests[requests[reqLast].nextIndex].host, host, strlen(host));
				memcpy(requests[requests[reqLast].nextIndex].page, page, strlen(page));
				requests[requests[reqLast].nextIndex].port = port;
				
				memset(requests[requests[reqLast].nextIndex].data, 0, teNetReqDataSize);
				memcpy(requests[requests[reqLast].nextIndex].data, data, dataSize);
				requests[requests[reqLast].nextIndex].dataSize = dataSize;
				requests[requests[reqLast].nextIndex].callback = callback;
				requests[requests[reqLast].nextIndex].userData = userData; 
				requests[requests[reqLast].nextIndex].waitRespType = waitRespType;
				if(filename)
					memcpy(requests[requests[reqLast].nextIndex].filename, filename, strlen(filename));
				else
					memset(requests[requests[reqLast].nextIndex].filename, 0, teNetReqSize);
				requests[requests[reqLast].nextIndex].failResCount = 0;
				requests[requests[reqLast].nextIndex].sended = 0;
				requests[requests[reqLast].nextIndex].error = E_NO_ERRORS;
				requests[requests[reqLast].nextIndex].commandsCount = netCommandsCount;
				memcpy(requests[requests[reqLast].nextIndex].commands, netCommands, sizeof(teNetCommand) * netCommandsCount);
				
				reqLast = requests[reqLast].nextIndex;
				reqCount++;
			}
			else
			{
				ClearRequests();
				
				memcpy(requests[0].host, host, strlen(host));
				memcpy(requests[0].page, page, strlen(page));
				requests[0].port = port;
				
				memset(requests[0].data, 0, teNetReqDataSize);
				memcpy(requests[0].data, data, dataSize);
				requests[0].dataSize = dataSize;
				requests[0].callback = callback;
				requests[0].userData = userData; 
				requests[0].waitRespType = waitRespType;
				if(filename)
					memcpy(requests[0].filename, filename, strlen(filename));
				else
					memset(requests[0].filename, 0, teNetReqSize);
				requests[0].failResCount = 0;
				requests[0].sended = 0;
				requests[0].error = E_NO_ERRORS;
				requests[0].commandsCount = netCommandsCount;
				memcpy(requests[0].commands, netCommands, sizeof(teNetCommand) * netCommandsCount);
				
				reqCount++;
			}
		}
		
		void teHTTPNetwork::AddToResponses(c8 * data, u32 dataSize, u32 errorID, u32 packetID, HTTPNetCallback callback, teptr_t userData)
		{
			if(respCount >= responses.GetSize()) return;
			
			if(respCount)
			{
				if(dataSize)
				{
					memset(responses[responses[respLast].nextIndex].data, 0, teNetRespSize);
					memcpy(responses[responses[respLast].nextIndex].data, data, dataSize);
				}
				responses[responses[respLast].nextIndex].dataSize = dataSize;
				responses[responses[respLast].nextIndex].packetID = packetID;
				responses[responses[respLast].nextIndex].error = errorID;
				responses[responses[respLast].nextIndex].callback = callback;
				responses[responses[respLast].nextIndex].userData = userData;
				
				respLast = responses[respLast].nextIndex;
				respCount++;
			}
			else
			{
				ClearResponses();
				if(dataSize)
				{
					memset(responses[0].data, 0, teNetRespSize);
					memcpy(responses[0].data, data, dataSize);
				}
				responses[0].dataSize = dataSize;
				responses[0].packetID = packetID;
				responses[0].error = errorID;
				responses[0].callback = callback;
				responses[0].userData = userData;
				
				respCount++;
			}
		}
				
		void teHTTPNetwork::DelFromRequests()
		{
			if(!reqCount) return;
			
			u32 temp = reqFirst;
			reqFirst = requests[reqFirst].nextIndex;
			
			requests[temp].nextIndex = requests[reqLast].nextIndex;
			requests[reqLast].nextIndex = temp;
			
			reqCount--;
		}
		
		void teHTTPNetwork::DelFromResponses()
		{
			if(!respCount) return;
			
			u32 temp = respFirst;
			respFirst = responses[respFirst].nextIndex;
			
			responses[temp].nextIndex = responses[respLast].nextIndex;
			responses[respLast].nextIndex = temp;
			
			respCount--;
		}
		
		u1 teHTTPNetwork::OpenSocket()
		{
			isConnected = 0;			
			socketID = socket( AF_INET, SOCK_STREAM, 0);
			if(socketID < 0)
			{
				TE_LOG("------ socket fail ! --------- ");
				TE_LOG("errno %i", errno);
				return 0;
			}
			
			linger lingerOpt;
			lingerOpt.l_onoff = 1;
			lingerOpt.l_onoff = 1;
			
			timeval tv;
			tv.tv_sec = 1;
			tv.tv_usec = 0 ;
			int result = setsockopt (socketID, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv, sizeof tv); 
			//int result = setsockopt(socketID, SOL_SOCKET, SO_LINGER, &lingerOpt, sizeof(lingerOpt));
			if (result < 0)
			{
				TE_LOG("------ socket fail ! --------- ");
				TE_LOG("errno %i", errno);
				//return 0;
			}
			
			return 1;
		}
		
		u1 teHTTPNetwork::IsRequestQueueComplete()
		{
			if(reqCount >= requests.GetSize())
				return 1;
			else
				return 0;
		}
		
		teHTTPNetwork * GetHTTPNetwork()
		{
			return currentHTTPNetwork;
		}
		
		void * ThreadRoutine(void * data)
		{
			teHTTPNetwork * tempHTTP;
			
			u32 sendNum = 0;
			u32 recvNum = 0;
			u32 lastSend = 0;
			
			while(1)
			{
				tempHTTP = GetHTTPNetwork();
				
				if(!tempHTTP)
				{
					teSleep(teNetSleepTime); 
					continue; 
				}
				
				if(!tempHTTP->OpenSocket())
				{
					teSleep(teNetSleepTime);
					continue; 
				}
				
//				if(tempHTTP->recvBufferUsed)
//					memset(tempHTTP->recvBuffer, 0, tempHTTP->recvBufferUsed);
				
				//------Send
				s32 sendResult = 0;
				teMutexLock(&tempMutex);
				u1 isRequest = 1;				
				if (tempHTTP->reqCount)
					if (!tempHTTP->requests[tempHTTP->reqFirst].sended)	
					{
						if(!tempHTTP->isConnected)
						{
							if (!tempHTTP->SetAddr(tempHTTP->requests[tempHTTP->reqFirst].host, tempHTTP->requests[tempHTTP->reqFirst].port)) //-- invalid adress or no internet connected
							{
								u32 tinvalid = E_INVALID_HOST;
								u32 temp = 0;
								
								tempHTTP->DelFromRequests();
									
								teMutexUnlock(&tempMutex);
								teSleep(teNetSleepTime);
								
								tempHTTP->Disconnect();
								continue; 
							}
								
							if(!tempHTTP->Connect())
							{
								teMutexUnlock(&tempMutex);
								teSleep(teNetSleepTime);
								
								tempHTTP->Disconnect();
								continue; 
							}
						}
						
						if(tempHTTP->requests[tempHTTP->reqFirst].sended)
						{
							teMutexUnlock(&tempMutex);
							teSleep(teNetSleepTime);
							
							tempHTTP->Disconnect();
							continue;
						}
						
						memset(tempHTTP->sendBuffer, 0, teNetSendBufferSize);
						tempHTTP->FormMessage(tempHTTP->requests[tempHTTP->reqFirst], tempHTTP->sendBuffer, tempHTTP->sendBufferUsed);
						
						sendResult = send(tempHTTP->socketID, tempHTTP->sendBuffer, tempHTTP->sendBufferUsed, 0);
						TE_LOG("---Send:%s", tempHTTP->sendBuffer)
						
						if(sendResult == tempHTTP->sendBufferUsed)
							tempHTTP->requests[tempHTTP->reqFirst].sended = 1;
					}
					else
					{
						isRequest = 0;
					}
				
				teMutexUnlock(&tempMutex);
				teSleep(teNetSleepTime);
				
				if((sendResult <= 0) || (!isRequest))
				{
					tempHTTP->Disconnect();
					continue;
				}
				
				//-------Receive
				s32 nbytes = 0;
				memset(tempHTTP->recvBuffer, 0, teNetRecvBufferSize);
				nbytes = recv(tempHTTP->socketID, tempHTTP->recvBuffer, teNetRecvBufferSize, 0);
				if(nbytes > 0)
				{
					u32 tNum = 0;
					while(nbytes > 0)
					{
						tNum += nbytes;
						
						if(nbytes > 2)
						{
							if(tempHTTP->recvBuffer[tNum + nbytes - 1] == '\n')
								if(tempHTTP->recvBuffer[tNum + nbytes - 2] == '\r')
									break;
						}
						else
							break;
						
						nbytes = recv(tempHTTP->socketID, &tempHTTP->recvBuffer[tNum] , teNetRecvBufferSize, 0);	
					}
					
					teMutexLock(&tempMutex);
					tempHTTP->Analyze(tNum);
					tempHTTP->DelFromRequests();
					teMutexUnlock(&tempMutex);
					
					tempHTTP->recvBufferUsed = tNum;
				}
				else
				{
					//---------  requests
					teMutexLock(&tempMutex);
					switch (tempHTTP->requests[tempHTTP->reqFirst].waitRespType)
					{
						case TE_NET_WAIT_RESPONSE_ETERNALY:
							break;
							
						case TE_NET_WAIT_RESPONSE_TIMEOUT:
							if(tempHTTP->requests[tempHTTP->reqFirst].failResCount >= teNetRespFailCount)
							{
								tempHTTP->DelFromRequests();
							}
							else
							{
								++tempHTTP->requests[tempHTTP->reqFirst].failResCount;
							}
							break;
							
						case TE_NET_WAIT_RESPONSE_ONCE:
							tempHTTP->AddToResponses(NULL, 0, E_TIMEOUT, 0, tempHTTP->requests[tempHTTP->reqFirst].callback, tempHTTP->requests[tempHTTP->reqFirst].userData);
							tempHTTP->DelFromRequests();
								
							break;	
					}
					teMutexUnlock(&tempMutex);
					nbytes = 0;
				}
				
				tempHTTP->Disconnect();
			}
			return NULL;	
		}
	}
}

#endif