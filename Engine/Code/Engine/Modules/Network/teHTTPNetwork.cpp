/*
 *  teHTTPNetwork.cpp
 *  TatEngine
 *
 *  Created by Dmitry Ivanov, Vadim Luchko on 05/10/13.
 *  Copyright 2013 Tatem Games. All rights reserved.
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
		u1 threadRun = true;

		#ifndef TE_PLATFORM_WIN

		pthread_t threadId;
		pthread_mutex_t threadMutex;

		void teMutexLock() {pthread_mutex_lock(&threadMutex);}
		void teMutexUnlock() {pthread_mutex_unlock(&threadMutex);}
		void teSleep(u32 milliseconds) {usleep(milliseconds * 1000);}

		#else

		u32 threadId;
		HANDLE threadMutex;

		void teHTTPMutexLock() {WaitForSingleObject(threadMutex, INFINITE);}
		void teHTTPMutexUnlock() {ReleaseMutex(threadMutex);}
		void teSleep(u32 milliseconds) {Sleep(milliseconds);}

		#endif

		// ------------------------------------------------------------------------------------------------------ URL

		void teHTTPUrl::Clear()
		{
			host = "";
			port = 0;
			path = "";
			query = "";
		}

		teHTTPUrl & teHTTPUrl::CopyFrom(const teHTTPUrl & other)
		{
			memcpy(buffer, other.buffer, sizeof(buffer));

			#define TE_MOVE_STR(__str) \
			{ \
				if((other.buffer <= other.__str.c_str()) && (other.buffer + sizeof(buffer) > other.__str.c_str())) \
					__str = teString(other.__str.c_str() - other.buffer + buffer); \
				else \
					__str = other.__str; \
			}

			TE_MOVE_STR(host);
			TE_MOVE_STR(path);
			TE_MOVE_STR(query);

			port = other.port;

			#undef TE_MOVE_STR

			return *this;
		}

		teHTTPUrl & teHTTPUrl::SetURI(const teString & setURI)
		{
			u32 i = 0;
			while((setURI.c_str()[i] == ' ') && (setURI.c_str()[i] != '\0')) ++i;

			u32 j = i;
			while((setURI.c_str()[j] != ' ') && (setURI.c_str()[j] != '\0')) ++j;

			if(j <= i + 1)
			{
				TE_LOG_ERR("teHTTPUrl empty");
				return *this;
			}

			u32 size = j - i;
			c8 temp[2 * 1024];

			if(size > sizeof(temp))
			{
				TE_LOG_ERR("teHTTPUrl URI too big");
				return *this;
			}

			memcpy(temp, setURI.c_str() + i, size);
			temp[size] = '\0';

			memset(buffer, 0, sizeof(buffer));
			teStringPool pool(buffer, sizeof(buffer));

			c8 * work = temp;

			c8 * isTwoSlsh = (c8*)strstr(work, "//");

			if(isTwoSlsh != NULL)
				work = isTwoSlsh + 2;

			c8 * isOneSlsh = (c8*)strchr(work, '/');

			if(isOneSlsh)
			{
				isOneSlsh[0] = '\0';

				c8 * isOneDblMrk = (c8*)strchr(work, ':');

				if(isOneDblMrk)
				{
					isOneDblMrk[0] = '\0';
					SetHTTP(pool.Clone(work).c_str(), atoi(isOneDblMrk + 1));
				}
				else
					SetHTTP(pool.Clone(work).c_str());

				isOneSlsh[0] = '/';

				work = isOneSlsh;

				c8 * isOneQstMark = (c8*)strchr(work, '?');

				if(isOneQstMark)
				{
					isOneQstMark[0] = '\0';
					SetPath(pool.Clone(work).c_str());
					isOneQstMark[0] = '?';

					work = isOneQstMark;

					SetQuery(pool.Clone(work + 1).c_str());
				}
				else
				{
					SetPath(pool.Clone(work).c_str());
				}
			}
			else
			{
				c8 * isOneDblMrk = (c8*)strchr(work, ':');

				if(isOneDblMrk)
				{
					isOneDblMrk[0] = '\0';
					SetHTTP(pool.Clone(work).c_str(), atoi(isOneDblMrk + 1));
				}
				else
					SetHTTP(pool.Clone(work).c_str());
			}
		}

		teHTTPUrl & teHTTPUrl::SetHTTP(const teString & setHost, u16 setPort)
		{
			host = setHost;
			path = "/";
			port = setPort;
			query = "";
			return *this;
		}

		teHTTPUrl & teHTTPUrl::SetPort(u16 setPort)
		{
			port = setPort;
			return *this;
		}

		teHTTPUrl & teHTTPUrl::SetPath(const teString & setPath)
		{
			path = setPath;
			return *this;
		}

		teHTTPUrl & teHTTPUrl::SetQuery(const teString & setQuery)
		{
			query = setQuery;
			return *this;
		}

		teHTTPUrl & teHTTPUrl::SetHandleData()
		{
			teHTTPUrl temp(*this);

			memset(buffer, 0, sizeof(buffer));
			teStringPool pool(buffer, sizeof(buffer));

			host = pool.Clone(temp.host);
			path = pool.Clone(temp.path);
			query = pool.Clone(temp.query);

			return *this;
		}

		#ifndef TE_PLATFORM_WIN
			#define TE_NET_ADR_TYPE sockaddr_in
		#else
			#define TE_NET_ADR_TYPE SOCKADDR_IN
		#endif

		u1 FillAdress(const teHTTPUrl & url, TE_NET_ADR_TYPE & netAddr)
		{
			struct hostent * host = NULL;

			#ifndef TE_PLATFORM_WIN
				bzero(&netAddr, sizeof(netAddr));
			#else
				memset(&netAddr,0, sizeof(netAddr));
			#endif

			netAddr.sin_family = AF_INET;
			netAddr.sin_port = htons(url.port);

			#ifndef TE_PLATFORM_WIN
			in_addr_t addrT = inet_addr(url.host.c_str());
			if(addrT != (in_addr_t)(-1))
			#else
			u32 addrT = inet_addr(url.host.c_str());
			if(addrT != INADDR_NONE)
			#endif
				netAddr.sin_addr.s_addr = addrT;
			else
			{
				host = gethostbyname(url.host.c_str());

				if(!host)
					return false;

				memcpy(&netAddr.sin_addr, host->h_addr_list[0], host->h_length);
			}

			return true;
		}

		// ------------------------------------------------------------------------------------------------------ Sockets

		void InitSocketSys()
		{
			#ifdef TE_PLATFORM_WIN
			WSADATA wsaData;
			if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
			{
				TE_LOG_ERR("teHTTPSocket::InitSocketSys() - WSAStartup failed: %s", WSAGetLastError());
				return;
			}
			#endif
		}

		void CloseSocketSys()
		{
			#ifdef TE_PLATFORM_WIN
			if(WSACleanup() != 0)
			{
				TE_LOG_ERR("teHTTPSocket::InitSocketSys() - WSACleanup failed: %s", WSAGetLastError());
				return;
			}
			#endif
		}

		teHTTPSocket::teHTTPSocket()
			:socketId(-1), connected(false)
		{
		}

		teHTTPSocket::~teHTTPSocket()
		{
			if(connected)
				Disconnect();
		}

		u1 teHTTPSocket::Connect(const teHTTPUrl & url)
		{
			if(connected)
				return true;

			TE_NET_ADR_TYPE netAddr;
			if(!FillAdress(url, netAddr))
				return false;

			if(socketId < 0)
			{
				socketId = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

				if(socketId < 0)
				{
					TE_LOG_ERR("teHTTPSocket::Connect() - socket creation failed: %i", errno);
					return false;
				}
			}

			if(connect(socketId, (struct sockaddr*)&netAddr, sizeof(netAddr)) != 0)
			{
				TE_LOG_ERR("teHTTPSocket::Connect() - connection fail: %i", errno);
				connected = false;
			}
			else
				connected = true;

			return connected;
		}

		void teHTTPSocket::Disconnect()
		{
			if(!connected)
				return;

			connected = false;

			#ifndef TE_PLATFORM_WIN
				if(shutdown(socketId, SHUT_RDWR) != 0)
				{
					TE_LOG_ERR("teHTTPSocket::Disconnect() - disconnect fail: %i", errno);
				}

				if(close(socketId) != 0)
				{
					TE_LOG_ERR("teHTTPSocket::Disconnect() - close fail: %i", errno);
				}
			#else
				if(closesocket(socketId) != 0)
				{
					TE_LOG_ERR("teHTTPSocket::Disconnect() - close fail: %i", errno);
				}
			#endif

			socketId = -1;
		}

		u1 teHTTPSocket::Write(const void * data, u32 dataSize)
		{
			u32 sent = 0;

			while(sent < dataSize)
			{
				s32 sendResult = send(socketId, (const c8*)data + (u32)sent, dataSize - (u32)sent, 0);

				if(sendResult < 0)
				{
					TE_LOG_ERR("teHTTPSocket::Write() - send fail: %i", errno);
					return false;
				}
				else
					sent += sendResult;
			}

			return true;
		}

		s32 teHTTPSocket::Read(void * buffer, u32 bufferSize)
		{
			return recv(socketId, (c8*)buffer, bufferSize, 0);
		}

		// ------------------------------------------------------------------------------------------------------ Requests

		teHTTPRequest::teHTTPRequest()
			:fileBuffer(NULL)
		{
			Clear();
		}

		teHTTPRequest::teHTTPRequest(const teString & setURI)
			:fileBuffer(NULL)
		{
			Clear();
			SetURL(teHTTPUrl(setURI));
		}

		teHTTPRequest::teHTTPRequest(const c8 * setURI)
			:fileBuffer(NULL)
		{
			Clear();
			SetURL(teHTTPUrl(setURI));
		}

		teHTTPRequest::teHTTPRequest(const teHTTPUrl & setURL)
			:fileBuffer(NULL)
		{
			Clear();
			SetURL(setURL);
		}

		teHTTPRequest::~teHTTPRequest()
		{
			Clear();
		}

		void teHTTPRequest::Clear()
		{
			url.Clear();
			type = RT_GET;
			postData = NULL;
			postDataSize = 0;
			callback = NULL;
			userData = 0;
			if(fileBuffer)
				fileBuffer->Unlock();
			TE_SAFE_DROP(fileBuffer);
			readBuffer = NULL;
			readBufferSize = 0;
			readBufferReadedSize = 0;
			mode = WM_ONCE;
			error = ET_NO_ERROR;
			errorsCount = 0;
			resendCount = 0;
			httpCode = 0;
			chunkSize = 0;
			chunkMode = false;
			sended = false;
			clear = true;
			readedHeader = false;
			redirected = false;
			resended = false;
			socket.Disconnect();

			GetHTTPNetwork()->GetDefaultBuffer(&readBuffer, readBufferSize);
		}

		void teHTTPRequest::PrepareToResend()
		{
			readBufferReadedSize = 0;
			error = ET_NO_ERROR;
			errorsCount = 0;
			httpCode = 0;
			chunkSize = 0;
			chunkMode = false;
			sended = false;
			readedHeader = false;
			resended = true;
			socket.Disconnect();
		}

		u1 teHTTPRequest::OpenFile()
		{
			if(strlen(fileName))
			{
				if(fileBuffer)
					return true;
				else
				{
					fileBuffer = core::GetFileManager()->OpenFile(fileName, core::CFileBuffer::FWM_WRITE);
					if(fileBuffer != NULL)
					{
						fileBuffer->Lock(core::BLT_WRITE);
						fileBuffer->SetStreamMode(true);
						fileBuffer->SetPosition(0);
					}
					else
					{
						TE_LOG_ERR("teHTTPRequest::OpenFile() - open failed (%s)", fileName);
					}
					return fileBuffer != NULL;
				}
			}
			else
				return true;
		}

		#ifdef TE_COMPILER_MSVC
		#define TE_SNPRINTF(...) \
			{ \
					s32 _res = sprintf_s(output + p, outputSize - p, __VA_ARGS__); \
				if(_res < 0) \
					return false; \
				else \
					p += _res; \
			}
		#else
		#define TE_SNPRINTF(...) \
			{ \
					s32 _res = snprintf(output + p, outputSize - p, __VA_ARGS__); \
				if(_res < 0) \
					return false; \
				else \
					p += _res; \
			}
		#endif

		u1 teHTTPRequest::FinalizeHeaders(c8 * output, u32 outputSize)
		{
			u32 p = 0;

			switch(type)
			{
			case RT_DELETE: TE_SNPRINTF("%s ", "DELETE"); break;
			case RT_POST: TE_SNPRINTF("%s ", "POST"); break;
			case RT_PUT: TE_SNPRINTF("%s ", "PUT"); break;
			case RT_GET:
			default:
				TE_SNPRINTF("%s ", "GET"); break;
			}

			TE_SNPRINTF("%s%s%s", url.path.c_str(), (url.query.GetLength() ? "?" : ""), url.query.c_str());

			if(postData)
			{
				TE_SNPRINTF(" HTTP/1.1\r\nHost:%s\r\nConnection:close\r\nAccept-Encoding:identity\r\nContent-Length:%u\r\nContent-Type:application/x-www-form-urlencoded\r\n%s\r\n", url.host.c_str(), postDataSize, (headers.c_str() ? headers.c_str() : ""));

				if((outputSize - p) < (postDataSize + 1))
					return false;

				memcpy(output + p, postData, postDataSize);
				p += postDataSize;
				output[p++] = '\0';
			}
			else
			{
				TE_SNPRINTF(" HTTP/1.1\r\nHost:%s\r\nConnection:close\r\nAccept-Encoding:identity\r\n%s\r\n", url.host.c_str(), (headers.c_str() ? headers.c_str() : ""));
			}

			return true;
		}

		EHTTPCallBackResult teHTTPRequest::Write(const void * data, u32 size)
		{
			EHTTPCallBackResult result = HCBR_OK;

			if(!data)
				return result;

			if(fileBuffer)
				fileBuffer->Write(data, size);

			if(readBuffer)
			{
				if((readBufferReadedSize + size) > readBufferSize)
				{
					if(callback)
						result = (*callback)(*this, readBuffer, readBufferReadedSize, true);

					readBufferReadedSize = 0;
				}

				memcpy(readBuffer + readBufferReadedSize, data, size);
				readBufferReadedSize += size;
			}
			else
				readBufferReadedSize += size;

			return result;
		}

		EHTTPCallBackResult teHTTPRequest::OnOk()
		{
			EHTTPCallBackResult result = HCBR_OK;
			if(callback)
				result = (*callback)(*this, readBuffer, readBufferReadedSize, mode == WM_KEEP_ALIVE);
			readBufferReadedSize = 0;
			return result;
		}

		EHTTPCallBackResult teHTTPRequest::OnError()
		{
			EHTTPCallBackResult result = HCBR_OK;
			if(callback)
				result = (*callback)(*this, NULL, -1, false);
			return result;
		}

		teHTTPRequest & teHTTPRequest::SetURL(const teHTTPUrl & setURL)
		{
			clear = false;
			url = setURL;
			return *this;
		}

		teHTTPRequest & teHTTPRequest::SetDelete()
		{
			type = RT_DELETE;
			return *this;
		}

		teHTTPRequest & teHTTPRequest::SetPost(const void * data, u32 dataSize)
		{
			type = RT_POST;

			if(dataSize <= sizeof(internalPostBuffer))
			{
				memcpy(internalPostBuffer, data, dataSize);
				postData = internalPostBuffer;
			}
			else
				postData = data;

			postDataSize = dataSize;

			return *this;
		}

		teHTTPRequest & teHTTPRequest::SetPost(teString data)
		{
			SetPost((const void*)data.GetRawRO(), data.GetSize());
			return *this;
		}

		teHTTPRequest & teHTTPRequest::SetPut(const void * data, u32 dataSize)
		{
			type = RT_PUT;

			if(dataSize <= sizeof(internalPostBuffer))
			{
				memcpy(internalPostBuffer, data, dataSize);
				postData = internalPostBuffer;
			}
			else
				postData = data;

			postDataSize = dataSize;

			return *this;
		}

		teHTTPRequest & teHTTPRequest::SetPut(teString data)
		{
			SetPut((const void*)data.GetRawRO(), data.GetSize());
			return *this;
		}

		teHTTPRequest & teHTTPRequest::SetHeaders(teString setHeaders)
		{
			headers = setHeaders;
			return *this;
		}

		teHTTPRequest & teHTTPRequest::SetUserData(teptr_t setUserData)
		{
			userData = setUserData;
			return *this;
		}

		teHTTPRequest & teHTTPRequest::SetSaveToFile(teString setFileName)
		{
			if(setFileName.GetSize() < sizeof(fileName))
				memcpy(fileName, setFileName.c_str(), setFileName.GetSize());
			return *this;
		}

		teHTTPRequest & teHTTPRequest::SetReadBuffer(c8 * buffer, u32 bufferSize)
		{
			readBuffer = buffer;
			readBufferSize = bufferSize;
			readBufferReadedSize = 0;
			return *this;
		}

		teHTTPRequest & teHTTPRequest::SetMode(EWorkMode setMode)
		{
			mode = setMode;
			return *this;
		}

		teHTTPRequest & teHTTPRequest::SetCallBack(teHTTPNetCallback setCallback)
		{
			callback = setCallback;
			return *this;
		}

		// ------------------------------------------------------------------------------------------------------ Network

		teHTTPNetwork * currentNetwork = NULL;

		teHTTPNetwork::teHTTPNetwork()
		{
			currentNetwork = this;

			InitSocketSys();

			#ifndef TE_PLATFORM_WIN
				pthread_mutex_init(&threadMutex, NULL);
				pthread_attr_t attr;
				pthread_attr_init(&attr);
				pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
				pthread_create(&threadId, &attr, (void *(*)(void*))teHTTPThreadRoutine, NULL);
				pthread_attr_destroy(&attr);
			#else
				threadMutex = CreateMutex(NULL, FALSE, NULL);
				_beginthread((void(_cdecl*)(void*))teHTTPThreadRoutine, 0, NULL);
			#endif

			requests.Reserve(128);
			requests.Request(requests.GetSize());

			for(u32 i = 0; i < requests.GetAlive(); ++i)
				requests[i].Clear();
		}

		teHTTPNetwork::~teHTTPNetwork()
		{
			threadRun = false;

			#ifndef TE_PLATFORM_WIN
				pthread_cancel(threadId);
				pthread_mutex_destroy(&threadMutex);
			#else
				CloseHandle((HANDLE)threadId);
			#endif

			for(u32 i = 0; i < requests.GetAlive(); ++i)
				requests[i].Clear();

			requests.Clear();

			CloseSocketSys();

			currentNetwork = NULL;
		}

		u32 teHTTPNetwork::Add(const teHTTPRequest & request)
		{
			teHTTPMutexLock();
			for(u32 i = 0; i < requests.GetAlive(); ++i)
				if(requests[i].clear)
				{
					requests[i] = request;
					teHTTPMutexUnlock();
					return i;
				}

			teHTTPMutexUnlock();
			return u32Max;
		}

		void teHTTPNetwork::Remove(u32 index)
		{
			if(index < requests.GetAlive())
			{
				teHTTPMutexLock();
				requests[index].Clear();
				teHTTPMutexUnlock();
			}
		}

		void teHTTPNetwork::Remove(const teHTTPRequest & request)
		{
			if(requests.IsFromThisArray(&request))
				Remove(requests.GetIndexInArray(&request));
		}

		void teHTTPNetwork::GetDefaultBuffer(c8 ** buffer, u32 & size)
		{
			if(!buffer)
				return;

			*buffer = defaultBuffer;
			size = sizeof(defaultBuffer);
		}

		u1 teHTTPNetwork::Process(teHTTPRequest & r, c8 * buffer, u32 bufferSize)
		{
			r.resended = false;

			if(!r.socket.connected)
			{
				if(!r.socket.Connect(r.url))
				{
					r.error = teHTTPRequest::ET_CONNECT_FAIL;
					return false;
				}
			}

			memset(buffer, 0, bufferSize);

			if(!r.FinalizeHeaders(buffer, bufferSize))
			{
				r.error = teHTTPRequest::ET_USER_HEADERS_TOO_BIG;
				return false;
			}

			if(!r.sended)
				r.sended = r.socket.Write(buffer);

			if(!r.sended)
			{
				r.error = teHTTPRequest::ET_SEND_FAIL;
				return false;
			}

			if(!r.OpenFile())
			{
				r.error = teHTTPRequest::ET_OPEN_FILE_FAIL;
				return false;
			}

			u1 overflow = true;
			u32 totalRecvSize = 0;
			while(overflow)
			{
				overflow = false;

				s32 recvSize = 0;
				while((recvSize = r.socket.Read(buffer + totalRecvSize, bufferSize - totalRecvSize)) > 0)
				{
					totalRecvSize += (u32)recvSize;
					if((bufferSize - totalRecvSize) < 1024)
					{
						overflow = true;
						break;
					}
				}

				if(totalRecvSize == 0)
					return true;

				c8 * result = buffer;
				u32 resultSize = (u32)totalRecvSize;

				totalRecvSize = 0;

				if(!r.readedHeader)
				{
					c8 * res = (c8*)strstr(buffer, "\r\n\r\n");

					if(res == NULL)
					{
					}
					else
					{
						res[2] = '\0'; // so buffer now contains c-string with http header

						const c8 * trEncoding = strstr(buffer, "Transfer-Encoding");

						if(trEncoding != NULL)
						{
							const c8 * trNextLine = strstr(trEncoding, "\r\n");
							const c8 * trChunkLine = strstr(trEncoding, "chunked");
							r.chunkMode = (trChunkLine != NULL) && (trNextLine != NULL) && (trChunkLine < trNextLine);
						}

						const c8 * isOkNextLine = strstr(buffer, "\r\n");
						const c8 * isOkHTTP11 = strstr(buffer, "HTTP/1.1");

						if((isOkNextLine != NULL) && (isOkHTTP11 == buffer) && (isOkNextLine - isOkHTTP11 > strlen("HTTP/1.1 ") + 3))
						{
							c8 temp[4] = {0, 0, 0, 0};
							memcpy(temp, buffer + strlen("HTTP/1.1 "), 3);
							r.httpCode = (u16)atoi(temp);
						}
						else
						{
							r.error = teHTTPRequest::ET_HTTP_HEADER_ERROR;
							return false;
						}

						if((r.httpCode > 300) && (r.httpCode < 399))
						{
							r.redirected = true;

							c8 * isOkLocation = strstr(buffer, "Location:");

							if(isOkLocation)
							{
								c8 * isOkLocationNextLine = strstr(isOkLocation, "\r\n");

								isOkLocation += strlen("Location:");
								isOkLocationNextLine[0] = '\0';

								r.PrepareToResend();
								r.url.SetURI(isOkLocation);
								return true;
							}
							else
							{
								r.error = teHTTPRequest::ET_HTTP_HEADER_ERROR;
								return false;
							}
						}
						else
							r.redirected = false;

						resultSize -= (res - buffer) + 4;
						result = res + 4;

						r.readedHeader = true;
					}
				}

				if(r.chunkMode)
				{
					if((r.chunkSize > 0) && (resultSize > 0))
					{
						if(r.chunkSize <= resultSize)
						{
							if(!ProcessResult(r, r.Write(result, r.chunkSize)))
								return true;

							result += r.chunkSize;
							resultSize -= r.chunkSize;

							r.chunkSize = 0;
						}
						else
						{
							if(!ProcessResult(r, r.Write(result, resultSize)))
								return true;

							r.chunkSize -= resultSize;

							result += resultSize;
							resultSize = 0;
						}
					}

					while((r.chunkSize == 0) && (resultSize > 0))
					{
						c8 * res = (c8*)strstr(result, "\r\n");

						if(res != NULL)
						{
							res[0] = '\0';
							sscanf(result, "%x", &r.chunkSize);

							resultSize -= (res - result) + 2;
							result = res + 2;
						}
						else
						{
							if(result > (buffer + resultSize)) // check if we can move
							{
								memmove(buffer, result, resultSize);
								totalRecvSize = resultSize;
								overflow = true;
								continue;
							}
							else
							{
								TE_LOG_ERR("http : unknown error while reading chunk size");
								r.error = teHTTPRequest::ET_UNKNOWN;
								return false;
							}
						}

						if(!r.chunkSize)
							continue;

						if(r.chunkSize <= resultSize)
						{
							if(!ProcessResult(r, r.Write(result, r.chunkSize)))
								return true;

							result += r.chunkSize;
							resultSize -= r.chunkSize;

							r.chunkSize = 0;
						}
						else
						{
							if(!ProcessResult(r, r.Write(result, resultSize)))
								return true;

							r.chunkSize -= resultSize;

							result += resultSize;
							resultSize = 0;
						}
					}

					if(r.chunkSize)
					{
						overflow = true;
						continue;
					}
				}
				else if(!ProcessResult(r, r.Write(result, resultSize)))
						return true;
			}

			return true;
		}

		u1 teHTTPNetwork::ProcessResult(teHTTPRequest & request, EHTTPCallBackResult result)
		{
			if(result == HCBR_OK)
				return true;
			else if(result == HCBR_TRY_SEND_AGAIN)
			{
				request.PrepareToResend();
				request.resendCount++;
				return false;
			}
			else if(result == HCBR_FORCE_CLOSE_CONNECTION)
			{
				GetHTTPNetwork()->Remove(request);
				return false;
			}
		}

		void * teHTTPNetwork::teHTTPThreadRoutine(void * data)
		{
			c8 buffer[teHTTPSocketReadBufferSize];
			teConstArray<teHTTPRequest> & reqs = GetHTTPNetwork()->Get();

			while(threadRun)
			{
				for(u32 i = 0; i < reqs.GetAlive(); ++i)
				{
					if(reqs[i].clear)
						continue;

					if(Process(reqs[i], buffer, sizeof(buffer)))
					{
						if(reqs[i].redirected || reqs[i].resended)
							continue;
						else
						{
							if(ProcessResult(reqs[i], reqs[i].OnOk()) && (reqs[i].mode != teHTTPRequest::WM_KEEP_ALIVE))
								GetHTTPNetwork()->Remove(i);
						}
					}
					else
					{
						++reqs[i].errorsCount;

						if((reqs[i].mode != teHTTPRequest::WM_TRYING) || (reqs[i].errorsCount > teHTTPMaxAttempts))
						{
							if(ProcessResult(reqs[i], reqs[i].OnError()))
								GetHTTPNetwork()->Remove(i);
						}
					}

					teSleep(teHTTPSleepTime);
				}

				teSleep(teHTTPSleepTime);
			}

			return NULL;
		}

		teHTTPNetwork * GetHTTPNetwork()
		{
			return currentNetwork;
		}
	}
}

#endif
