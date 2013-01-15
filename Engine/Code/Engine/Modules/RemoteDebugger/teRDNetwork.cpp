/*
 *  teRDNetwork.cpp
 *  TatEngine
 *
 *  Created by Dmitrii Ivanov on 07/1/12.
 *  Copyright 2012 Tatem Games. All rights reserved.
 *
 */

#include "zmq.hpp"
#include "teRDNetwork.h"
#include "teLogManager.h"

namespace te
{
	namespace dbg
	{
		teRDNetwork::teRDNetwork()
			:zmqContext(NULL), zmqCmd(NULL), zmqDataDrop(NULL)
		{
		}

		teRDNetwork::~teRDNetwork()
		{
			StopServer();
		}

		void teRDNetwork::StartServer(u1 asClient, const c8 * connectToIP)
		{
			if(zmqContext)
				StopServer();

			zmqContext = zmq_init(1);

			if(asClient)
			{
				c8 temp[256];

				zmqCmd = zmq_socket(zmqContext, ZMQ_REQ);
				sprintf(temp, "tcp://%s:7748", connectToIP);
				zmq_connect(zmqCmd, temp);

				zmqDataDrop = zmq_socket(zmqContext, ZMQ_PULL);
				sprintf(temp, "tcp://%s:7749", connectToIP);
				zmq_connect(zmqDataDrop, temp);
			}
			else
			{
				zmqCmd = zmq_socket(zmqContext, ZMQ_REP);
				zmq_bind(zmqCmd, "tcp://*:7748");

				zmqDataDrop = zmq_socket(zmqContext, ZMQ_PUSH);
				zmq_bind(zmqDataDrop, "tcp://*:7749");
			}
		}

		void teRDNetwork::StopServer()
		{
			if(!zmqContext)
				return;

			zmq_close(zmqDataDrop);
			zmq_close(zmqCmd);
			zmq_term(zmqContext);

			zmqDataDrop = NULL;
			zmqCmd = NULL;
			zmqContext = NULL;
		}

		void teRDNetwork::Send(u32 type, u32 * data, f32 * floatData)
		{
			if(!zmqContext)
				return;

			zmq_msg_t reply;
			zmq_msg_init_size(&reply, sizeof(tePkg));

			tePkg * pkg = (tePkg*)zmq_msg_data(&reply);

			const c8 * magic = "TAT2";

			pkg->magic = *(u32*)magic;
			pkg->type = type;

			if(data)
				memcpy(pkg->d, data, 4 * sizeof(u32));

			if(floatData)
				memcpy(pkg->f, floatData, 4 * sizeof(f32));

			zmq_send(zmqCmd, &reply, 0);
			zmq_msg_close(&reply);
		}

		zmq_msg_t getRequest;

		tePkg * teRDNetwork::Get(u1 nonBlock)
		{
			if(!zmqContext)
				return NULL;

			zmq_msg_init(&getRequest);
			s32 errCode = zmq_recv(zmqCmd, &getRequest, (nonBlock ? ZMQ_NOBLOCK : 0));
			if(errCode != 0)
			{
				if(zmq_errno() != EAGAIN)
					TE_LOG_WRN("0mq recv err %i : %i", errCode, zmq_errno());

				return NULL;
			}

			if(zmq_msg_size(&getRequest) != sizeof(tePkg))
			{
				TE_LOG_WRN("0mq packet wrong size %1 != %2", zmq_msg_size(&getRequest), sizeof(tePkg));
				return NULL;
			}

			tePkg * pkg = (tePkg*)zmq_msg_data(&getRequest);

			const c8 * magic = "TAT2";

			if(pkg->magic != *(u32*)magic)
			{
				TE_LOG_WRN("0mq packet wrong magic");
				return NULL;
			}

			return pkg;
		}

		void teRDNetwork::ClearGet()
		{
			if(!zmqContext)
				return;

			zmq_close(&getRequest);
		}

		void teRDNetwork::SendData(const void * data, u32 dataSize)
		{
			if(!zmqContext)
				return;

			zmq_msg_t reply;
			zmq_msg_init_size(&reply, dataSize);
			memcpy(zmq_msg_data(&reply), data, dataSize);
			zmq_send(zmqDataDrop, &reply, 0);
			zmq_msg_close(&reply);
		}

		zmq_msg_t getDataRequest;

		void * teRDNetwork::GetData(u32 & dataSize)
		{
			if(!zmqContext)
				return NULL;

			zmq_msg_init(&getDataRequest);
			s32 errCode = zmq_recv(zmqDataDrop, &getDataRequest, 0);
			if(errCode != 0)
			{
				if(zmq_errno() != EAGAIN)
					TE_LOG_WRN("0mq recv data err %i : %i", errCode, zmq_errno());

				return NULL;
			}

			dataSize = zmq_msg_size(&getDataRequest);

			return zmq_msg_data(&getDataRequest);
		}

		void teRDNetwork::ClearGetData()
		{
			if(!zmqContext)
				return;

			zmq_close(&getDataRequest);
		}
	}
}
