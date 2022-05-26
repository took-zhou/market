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
                tickDataPool tempData;
                tempData.id.exch = readData[iter.key()]["instrument"][i]["id"]["exch"];
                tempData.id.ins = readData[iter.key()]["instrument"][i]["id"]["ins"];
                tempData.id.ticksize = utils::stringToFloat(readData[iter.key()]["instrument"][i]["id"]["ticksize"]);
                tempData.index = readData[iter.key()]["instrument"][i]["index"];
                tempControl.instrumentList.insert(tempData);
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

    // 更新InstrumentInfo
    updatePublishInstrumentInfo();

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
            ins_exch["id"]["exch"] = ins_iter->id.exch;
            ins_exch["id"]["ins"] = ins_iter->id.ins;
            ins_exch["id"]["ticksize"] = utils::floatToStringConvert(ins_iter->id.ticksize);
            ins_exch["index"] = ins_iter->index;
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
}

void controlPara::setDirectForwardingFlag(const std::string keyname, bool flag)
{
    std::map<std::string, publishControl>::iterator iter = publishCtrlMap.find(keyname);
    if (iter != publishCtrlMap.end())
    {
        iter->second.directforward = flag;
    }
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
}

void controlPara::buildInstrumentList(const std::string keyname, std::vector<utils::InstrumtntID> const &nameVec)
{
    std::map<std::string, publishControl>::iterator iter = publishCtrlMap.find(keyname);
    if (iter == publishCtrlMap.end())
    {
        publishControl tempControl;
        for (int i = 0; i < nameVec.size(); i++)
        {
            tickDataPool tempData;
            tempData.id = nameVec[i];
            auto pos = instrumentList.find(tempData);
            if (pos == end(instrumentList))
            {
                tempData.index = instrumentList.size();
                instrumentList.insert(tempData);
            }
            else
            {
                tempData.index = pos->index;
            }
            tempControl.instrumentList.insert(tempData);
        }

        INFO_LOG("insert keyname: %s", keyname.c_str());
        publishCtrlMap.insert(make_pair(keyname, tempControl));
    }
}

std::vector<utils::InstrumtntID> controlPara::getInstrumentList(void)
{
    std::vector<utils::InstrumtntID> instrument_vector;
    instrument_vector.clear();
    auto iter = instrumentList.begin();
    while (iter != instrumentList.end())
    {
        instrument_vector.push_back(iter->id);
        iter++;
    }

    return instrument_vector;
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

void controlPara::updatePublishInstrumentInfo(void)
{
    instrumentList.clear();
    std::map<string, publishControl>::iterator iter;
    U32 index_count = 0;
    for (iter = publishCtrlMap.begin(); iter != publishCtrlMap.end(); iter++)
    {
        auto iter2 = iter->second.instrumentList.begin();
        while (iter2 != iter->second.instrumentList.end())
        {
            iter2->index = index_count;
            instrumentList.insert(*iter2);

            index_count++;
            iter2++;
        }
    }
}
