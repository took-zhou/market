/*
 * innerZmq.cpp
 *
 *  Created on: 2021年2月17日
 *      Author: Administrator
 */

#include "market/infra/inner_zmq.h"

InnerZmq::InnerZmq() { context = zmq_ctx_new(); }
