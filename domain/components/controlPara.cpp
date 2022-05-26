#include <fstream>
#include <iomanip>

#include "market/domain/components/controlPara.h"

#include "common/extern/log/log.h"
#include "common/extern/json/json.h"
#include "common/extern/json/fifo_map.hpp"
#include "common/self/fileUtil.h"
#include "common/self/utils.h"

using namespace nlohmann;
template<class K, class V, class dummy_compare, class A>
using fifo_workaround_fifo_map = fifo_map<K, V, fifo_map_compare<K>, A>;
using fifo_json  = basic_json<fifo_workaround_fifo_map>;

controlPara::controlPara(void)
{
    auto& jsonCfg = utils::JsonConfig::getInstance();
    json_path = jsonCfg.getConfig("market", "ControlParaFilePath").get<std::string>();
    if (!utils::isFileExist(json_path))
    {
        utils::creatFile(json_path);
    }
    else if(utils::getFileSize(json_path) > 0)
    {
        load_from_json();
    }
}

bool controlPara::load_from_json(void)
{
    int ret = true;
    fifo_json readData;
    ifstream outFile(json_path, ios::binary);
    if (outFile.is_open())
    {
        outFile >> readData;
        for (auto iter = readData.begin(); iter != readData.end(); iter++)
        {
            publishControl tempControl;
            for (int i = 0; i < readData[iter.key()]["instrument"].size(); i++)
            {
                utils::InstrumtntID id;
                id.exch = readData[iter.key()]["instrument"][i]["exch"];
                id.ins = readData[iter.key()]["instrument"][i]["ins"];
                id.ticksize = utils::stringToFloat(readData[iter.key()]["instrument"][i]["ticksize"]);
                tempControl.instrumentList.insert(id);
            }

            readData[iter.key()].at("indication").get_to(tempControl.indication);
            readData[iter.key()].at("interval").get_to(tempControl.interval);
            readData[iter.key()].at("directforward").get_to(tempControl.directforward);
            readData[iter.key()].at("source").get_to(tempControl.source);
            readData[iter.key()].at("thread_uniqueness_cnt").get_to(tempControl.thread_uniqueness_cnt);

            INFO_LOG("load keyname: %s", iter.key().c_str());
            publishCtrlMap.insert(make_pair(iter.key(), tempControl));
        }
    }
    else
    {
        WARNING_LOG("file:%s not exist.", json_path.c_str());
        ret = false;
    }
    outFile.close();

    return ret;
}

bool controlPara::write_to_json(void)
{
    int ret = true;
    fifo_json writeData;

    std::map<std::string, publishControl>::iterator mapit = publishCtrlMap.begin();
    while (mapit  != publishCtrlMap.end())
    {
        fifo_json one_item;
        auto ins_iter = mapit->second.instrumentList.begin();
        while (ins_iter != mapit->second.instrumentList.end())
        {
            fifo_json ins_exch;
            ins_exch["exch"] = ins_iter->exch;
            ins_exch["ins"] = ins_iter->ins;
            ins_exch["ticksize"] = utils::floatToStringConvert(ins_iter->ticksize);
            one_item["instrument"].push_back(ins_exch);
            ins_iter++;
        }

        one_item["indication"] = mapit->second.indication;
        one_item["interval"] = mapit->second.interval;
        one_item["source"] = mapit->second.source;
        one_item["directforward"] = mapit->second.directforward;
        one_item["thread_uniqueness_cnt"] = mapit->second.thread_uniqueness_cnt;

        writeData[mapit->first] = one_item;
        mapit++;
    }

    ofstream inFile(json_path);
    if( inFile.is_open() )
    {
        inFile << setw(4) << writeData<< endl;
    }
    else
    {
        ERROR_LOG("file:%s open error.", json_path.c_str());
        ret = false;
    }
    inFile.close();

    return ret;
}

void controlPara::setInterval(const std::string keyname, float _interval)
{
    std::map<std::string, publishControl>::iterator iter = publishCtrlMap.find(keyname);
    if (iter != publishCtrlMap.end())
    {
        iter->second.interval = (U32)(_interval*1000000);
    }
    write_to_json();
}

void controlPara::setDirectForwardingFlag(const std::string keyname, bool flag)
{
    std::map<std::string, publishControl>::iterator iter = publishCtrlMap.find(keyname);
    if (iter != publishCtrlMap.end())
    {
        iter->second.directforward = flag;
    }
    write_to_json();
}

void controlPara::setStartStopIndication(const std::string keyname, market_strategy::TickStartStopIndication_MessageType _indication)
{
    std::map<std::string, publishControl>::iterator iter = publishCtrlMap.find(keyname);
    if (iter != publishCtrlMap.end())
    {
        iter->second.indication = _indication;
        INFO_LOG("setStartStopIndication %d.", _indication);
    }
    else
    {
        ERROR_LOG("Please first send tickdatareq action, keyname: %s.", keyname.c_str());
    }
    write_to_json();
}

void controlPara::setSource(const std::string keyname, std::string _source)
{
    std::map<std::string, publishControl>::iterator iter = publishCtrlMap.find(keyname);
    if (iter != publishCtrlMap.end())
    {
        iter->second.source = _source;
    }
    else
    {
        ERROR_LOG("Please first send tickdatareq action, keyname: %s.", keyname.c_str());
    }
    write_to_json();
}

void controlPara::buildInstrumentList(const std::string keyname, std::vector<utils::InstrumtntID> const &nameVec)
{
    std::map<std::string, publishControl>::iterator iter = publishCtrlMap.find(keyname);
    if (iter == publishCtrlMap.end())
    {
        publishControl tempControl;
        for (int i = 0; i < nameVec.size(); i++)
        {
            tempControl.instrumentList.insert(nameVec[i]);
        }

        INFO_LOG("insert keyname: %s", keyname.c_str());
        publishCtrlMap.insert(make_pair(keyname, tempControl));
    }

    write_to_json();
}

void controlPara::eraseInstrumentList(const std::string keyname)
{
    std::map<string, publishControl>::iterator iter = publishCtrlMap.find(keyname);
    if (iter != publishCtrlMap.end())
    {
        INFO_LOG("%s req alive timeout, will not subscribe.", iter->first.c_str());
        publishCtrlMap.erase(iter);
    }

    write_to_json();
}

std::vector<utils::InstrumtntID> controlPara::getInstrumentList(void)
{
    std::vector<utils::InstrumtntID> instrument_vec;
    instrument_vec.clear();
    std::map<std::string, publishControl>::iterator iter;

    for (iter = publishCtrlMap.begin(); iter != publishCtrlMap.end(); iter++)
    {
        auto ins_iter = iter->second.instrumentList.begin();
        while (ins_iter != iter->second.instrumentList.end())
        {
            instrument_vec.push_back(*ins_iter);
            ins_iter++;
        }
    }

    return instrument_vec;
}

std::vector<std::string> controlPara::getKeyNameList(void)
{
    std::vector<std::string> temp_vec;
    temp_vec.clear();
    std::map<std::string, publishControl>::iterator iter;

    for (iter = publishCtrlMap.begin(); iter != publishCtrlMap.end(); iter++)
    {
        temp_vec.push_back(iter->first);
    }

    return temp_vec;
}
