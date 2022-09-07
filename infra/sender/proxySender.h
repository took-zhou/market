/*
 * proxySender.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_PROXYSENDER_H_
#define WORKSPACE_MARKET_INFRA_PROXYSENDER_H_

#include <string>
#include "common/self/utils.h"

struct ProxySender {
  ProxySender(){};
  bool send(utils::ItpMsg &msg);
};

#endif /* WORKSPACE_MARKET_INFRA_PROXYSENDER_H_ */
