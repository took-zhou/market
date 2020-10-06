/*
 * define.cpp
 *
 *  Created on: 2020Äê9ÔÂ9ÈÕ
 *      Author: Administrator
 */

#include "market/infra/define.h"
#include <map>
#include "common/extern/libgo/libgo/libgo.h"

std::map<std::string, EventType> TitleToEvent;

static MsgStruct NilMsgStruct;

co_chan<MsgStruct> ctpMsgChan;




