#!/usr/bin/python
import csv
import os, os.path
import sys
import random

logger_dir = os.path.expanduser("~/.local/share/TpLogger/logs/ofono_ofono_account0")

xml_header = """<?xml version='1.0' encoding='utf-8'?>
<?xml-stylesheet type="text/xsl" href="log-store-xml.xsl"?>
<log>
"""
xml_footer = "</log>"

sms = {}
call = {}

if len(sys.argv) != 2 or not os.path.isfile(sys.argv[1]):
    print "Usage: " + sys.argv[0] + " conversation_data.csv"
    sys.exit(1)

with open(sys.argv[1], 'rb') as csvfile:
    reader = csv.reader(csvfile)
    count = 0
    for row in reader:
        count+=1
        # skip header
        if count == 1:
            continue
        phoneNumber = row[0]
        datetime = row[1]
        eventtype = row[2]
        incoming = (row[3] == "yes")
        source = row[4]
        missed = (row[5] == "yes")
        message = row[6]
        duration = row[7]
        # use phoneNumber as userid
        userid = phoneNumber.replace(" ", "") # no space
        userid = userid.replace("(", "").replace(")","") # no ()
        userid = userid.replace("-", "") # no dash

        # fix possible format errors
        date, time = datetime.split(" ")
        if len(time) == 7:
            time = "0"+time

        day, month, year =  date.split("/")
        if len(month) == 1:
            month = "0"+month

        if len(day) == 1:
            day = "0"+day

        filedate = year+month+day
        finalDate = filedate+"T"+time

        token = str(random.randrange(111111111, 999999999))

        if eventtype == "sms":
            string = None
            if incoming:
                string = "<message time='"+finalDate+"' id='"+userid+"' name='"+phoneNumber+"' token='' isuser='false' type='normal' message-token='"+token+"'>"+message+"</message>\n"
            else:
                string = "<message time='"+finalDate+"' id='ofono' name='ofono' token='' isuser='true' type='normal'>"+message+"</message>\n"

            if userid in sms:
                if filedate in sms[userid]:
                    sms[userid][filedate] += string
                else:
                    sms[userid][filedate] = string
            else:
                sms[userid] = {filedate: string}

        if eventtype == "call":
            string = None
            if incoming:
                if missed:
                    string = "<call time='"+finalDate+"' id='"+userid+"' name='"+phoneNumber+"' isuser='false' token='' duration='-1' actor='"+userid+"' actortype='contact' actorname='"+phoneNumber+"' actortoken='' reason='no-answer' detail=''/>\n"
                else:
                    string = "<call time='"+finalDate+"' id='"+userid+"' name='"+phoneNumber+"' isuser='false' token='' duration='"+duration+"' actor='"+userid+"' actortype='contact' actorname='"+phoneNumber+"' actortoken='' reason='user-requested' detail=''/>\n"
            else:
                string = "<call time='"+finalDate+"' id='ofono' name='ofono' isuser='true' token='' duration='"+duration+"' actor='"+userid+"' actortype='contact' actorname='"+phoneNumber+"' actortoken='' reason='user-requested' detail=''/>\n"

            if userid in call:
                if filedate in call[userid]:
                    call[userid][filedate] += string
                else:
                    call[userid][filedate] = string
            else:
                call[userid] = {filedate: string}

if len(call) != 0 or len(sms) != 0:
    # create sms log files
    for userid in sms:
        user_sms_dir = logger_dir+"/"+userid
        if not os.path.exists(user_sms_dir):
            os.makedirs(user_sms_dir)
        for date in sms[userid]:
            f = open(user_sms_dir+"/"+date+".log", 'w')
            f.write(xml_header)
            f.write(sms[userid][date])
            f.write(xml_footer)
            f.close()

    # create call log files
    for userid in call:
        user_call_dir = logger_dir+"/"+userid
        if not os.path.exists(user_call_dir):
            os.makedirs(user_call_dir)
        for date in call[userid]:
            f = open(user_sms_dir+"/"+date+".call.log", 'w')
            f.write(xml_header)
            f.write(call[userid][date])
            f.write(xml_footer)
            f.close()
    print "Done! logger database populated at: " + logger_dir
else:
    print "No data found"

