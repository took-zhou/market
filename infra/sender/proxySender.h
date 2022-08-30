/*
 * proxySender.h
 *
 *  Created on: 2020.11.13
 *      Author: Administrator
 */

#ifndef WORKSPACE_MARKET_INFRA_PROXYSENDER_H_
#define WORKSPACE_MARKET_INFRA_PROXYSENDER_H_

struct ProxySender {
  ProxySender(){};
  bool send(const char *head, const char *msg);
};

#endif /* WORKSPACE_MARKET_INFRA_PROXYSENDER_H_ */
