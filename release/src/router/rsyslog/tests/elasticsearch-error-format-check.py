import json
import sys
import os
def checkDefaultErrorFile():
    with open(os.environ['RSYSLOG_DYNNAME'] + ".errorfile") as json_file:
        json_data = json.load(json_file)
        indexCount =0
        replyCount=0
        for item in json_data:
            if item == "request":
                for reqItem in json_data[item]:
                    if reqItem == "url":
                        print "url found"
                        print reqItem
                    elif reqItem == "postdata":
                        print "postdata found"
                        indexCount = str(json_data[item]).count('\"_index\":')
                        print reqItem
                    else:
                        print reqItem
                        print "Unknown item found"
                        sys.exit(1)
                
            elif item == "reply":
                for replyItem in json_data[item]:
                    if replyItem == "items":
                        print json_data[item][replyItem]
                        replyCount = str(json_data[item][replyItem]).count('_index')
                    elif replyItem == "errors":
                        print "error node found"
                    elif replyItem == "took":
                        print "took node found"
                    else:
                        print replyItem
                        print "Unknown item found"
                        sys.exit(3)

            else:
                print item
                print "Unknown item found"
                print "error"
                sys.exit(4)
    if replyCount == indexCount :
        return 0
    else:
        sys.exit(7)
    return 0


def checkErrorOnlyFile():
    with open(os.environ['RSYSLOG_DYNNAME'] + ".errorfile") as json_file:
        json_data = json.load(json_file)
        indexCount =0
        replyCount=0
        for item in json_data:
            if item == "request":
                print json_data[item]
                indexCount = str(json_data[item]).count('\"_index\":')
                
                
            elif item == "url":
                print "url found"
                
            
            elif item == "reply":
                print json_data[item]
                replyCount = str(json_data[item]).count('\"_index\":')

            else:
                print item
                print "Unknown item found"
                print "error"
                sys.exit(4)
    if replyCount == indexCount :
        return 0
    else:
        sys.exit(7)
    return 0

def checkErrorInterleaved():
    with open(os.environ['RSYSLOG_DYNNAME'] + ".errorfile") as json_file:
        json_data = json.load(json_file)
        indexCount =0
        replyCount=0
        for item in json_data:
            print item
            if item == "response":
                for responseItem in json_data[item]:
                    print responseItem
                    for res in responseItem:
                        print res
                        if res == "request":
                            print responseItem[res]
                            indexCount = str(responseItem[res]).count('\"_index\":')
                            print "request count ", indexCount
                        elif res == "reply":
                            print responseItem[res]
                            replyCount = str(responseItem[res]).count('\"_index\":')
                            print "reply count ", replyCount
                        else:
                            print res
                            print "Unknown item found"
                            sys.exit(9)
                    if replyCount != indexCount :
                        sys.exit(8)
            
                
                
            elif item == "url":
                print "url found"
                
            
           

            else:
                print item
                print "Unknown item found"
                sys.exit(4)
    
    return 0

def checkInterleaved():
    return checkErrorInterleaved()

if __name__ == "__main__":
    option = sys.argv[1]
    if option == "default":
        checkDefaultErrorFile()
    elif option == "erroronly":
        checkErrorOnlyFile()
    elif option == "errorinterleaved":
        checkErrorInterleaved()
    elif option == "interleaved":
        checkErrorInterleaved()
    else:
        print "Usage: <script> <default|erroronly|errorinterleaved>"
        sys.exit(6)
