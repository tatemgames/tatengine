/*
 *  teRDNetwork.h
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 07/1/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#ifndef TE_TERDNETWORK_H
#define TE_TERDNETWORK_H

#include "teTypes.h"
#include "teConstArray.h"

namespace te
{
	namespace dbg
	{
		#pragma pack(push, 1)

		struct tePkg
		{
			u32 magic;
			u32 type;
			u32 d[4];
			f32 f[4];
		};

		#pragma pack(pop)

		//struct zmq_msg_t;

		class teRDNetwork
		{
		public:
			teRDNetwork();
			~teRDNetwork();

			void StartServer(u1 asClient = false, const c8 * connectToIP = NULL);
			void StopServer();
			void RestartServer()
			{
				StopServer();
				StartServer();
			}

			void Send(u32 type, u32 * data = NULL, f32 * floatData = NULL);
			tePkg * Get(u1 nonBlock = true);
			void ClearGet();

			void SendData(const void * data, u32 dataSize);
			void * GetData(u32 & dataSize);
			void ClearGetData();

			template<typename T>
			void SendData(const teConstArray<T> & constArray) {SendData(constArray.GetPool(), constArray.GetAliveBytes());}

		protected:
			void * zmqContext;
			void * zmqCmd;
			void * zmqDataDrop;
		};
	}
}

#endif
