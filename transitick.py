#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import smtplib
from email.mime.text import MIMEText
from email.utils import formataddr
import datetime
import threading
import time
import os
import glob
import logging

logging.basicConfig(level=logging.DEBUG,
                    filename='transitick.log',
                    filemode='a',
                    format=
                    '%(asctime)s - %(pathname)s[line:%(lineno)d] - %(levelname)s: %(message)s'
                    #日志格式
                    )

my_sender='zhoufan@cdsslh.com'
my_pass = 'sCARbo12'
my_user='zhoufan@cdsslh.com'
pack_path='/etc/chaodai/history_data_from_ctp'
temp_store_path='/etc/chaodai/history_data_from_ctp_temp'
day_transit_start_hour = 16
day_transit_end_hour =  20
day_transit_check_hour =  21

night_transit_start_hour = 4
night_transit_end_hour = 8
night_transit_check_hour = 9

complete_transit_flag = 1

def during_day_transit_time():
    out = False
    n_time_hour = time.localtime().tm_hour
    if (n_time_hour >= day_transit_start_hour and n_time_hour <= day_transit_end_hour):
        out = True
    return out

def during_night_transit_time():
    out = False
    n_time_hour = time.localtime().tm_hour
    if (n_time_hour >= night_transit_start_hour and n_time_hour <= night_transit_end_hour):
        out = True
    return out

def during_day_check_complete_time():
    out = False
    n_time_hour = time.localtime().tm_hour
    if (n_time_hour >= day_transit_end_hour and n_time_hour <= day_transit_check_hour):
        out = True
    return out

def during_night_check_complete_time():
    out = False
    n_time_hour = time.localtime().tm_hour
    if (n_time_hour >= night_transit_end_hour and n_time_hour <= night_transit_check_hour):
        out = True
    return out

def mail(msgbody):
    ret=True
    try:
        msg=MIMEText(msgbody,'plain','utf-8')
        msg['From']=formataddr(["zhoufan",my_sender])
        msg['To']=formataddr(["zhoufan",my_user])
        msg['Subject']="每日tick数据采集"

        server=smtplib.SMTP_SSL("smtp.exmail.qq.com", 465)

        server.login(my_sender, my_pass)
        server.sendmail(my_sender,[my_user,],msg.as_string())
        server.quit()
        logging.debug("邮件发送成功")
    except Exception:
        logging.debug("邮件发送失败")
        ret=False
    return ret

def pack_tick_data():
    os.chdir(pack_path)
    path_file_number=glob.glob(pathname='*.csv')
    if len(path_file_number) == 0:
        logging.debug("no csv file found.")
        out = -1
    else:
        today = datetime.date.today()
        pack_name='history_tick_%s.tar.gz'%(today)
        command_str = 'tar -czf %s *'%(pack_name)
        logging.debug(command_str)
        os.system(command_str)
        out = 0
    return out

def to_baiduyun():
    today = datetime.date.today()
    pack_name='history_tick_%s.tar.gz'%(today)
    os.chdir(pack_path)
    if os.path.exists(pack_name):
        if during_day_transit_time():
            command_str = 'bypy upload %s /tick/day/%s'%(pack_name, pack_name)
        elif during_night_transit_time():
            command_str = 'bypy upload %s /tick/night/%s'%(pack_name, pack_name)
        else:
            logging.error("不在正确的时间内上传")
        logging.debug(command_str)
        os.system(command_str)

def handle_after_transit(transitime):
    today = datetime.date.today()
    pack_name='history_tick_%s.tar.gz'%(today)
    os.chdir(pack_path)
    fsize = os.path.getsize(pack_name)
    fsize = fsize/float(1024 * 1024)
    fsize = round(fsize, 2)
    if os.getcwd() == pack_path:
        command_str = 'rm -rf *'
        os.system(command_str)
    else:
        logging.error("当前路径错误，请及时查看")

    msgbody='今日tick数据上传到百度网盘成功，文件名：%s，大小：%f M；\
            传输用时：%d 秒'%(pack_name, fsize, transitime)
    logging.debug(msgbody)
    mail(msgbody)

def transit_tick():
    global complete_transit_flag
    one_go_count = 0
    while(1):
        if during_day_transit_time() or during_night_transit_time():
            one_go_count = one_go_count + 1
            time.sleep(60)

            if one_go_count == 1:
                logging.debug(complete_transit_flag)
                complete_transit_flag = 0
                if pack_tick_data() == 0:
                    starttime = datetime.datetime.now()
                    to_baiduyun()
                    endtime = datetime.datetime.now()
                    uesd_second = (endtime - starttime).seconds
                    handle_after_transit(uesd_second)
                else:
                    logging.debug("transit operation is end")
                complete_transit_flag = 1
        else:
            one_go_count = 0
            time.sleep(60)

def handle_transit_fail():
    command_str = 'cp -f %s %s'%(pack_path, temp_store_path)
    os.system(command_str)
    msgbody='tick数据传输失败，已将数据临时拷贝到目录：%s，\
            请紧急处理。'%(temp_store_path)
    logging.debug(msgbody)
    mail(msgbody)
    exit -1

def check_transit_complete():
    global complete_transit_flag
    while(1):
        if (during_day_check_complete_time() or during_night_check_complete_time()) and complete_transit_flag == 0:
            handle_transit_fail()
        else:
            time.sleep(60)

if __name__ == "__main__":
    logging.debug('tick transit starting...')
    t1 = threading.Thread(target=transit_tick)
    t2 = threading.Thread(target=check_transit_complete)
    t1.start()
    t2.start()
