#ifndef HOMEALARMIOSIMULATORVXWORKSDRV_H
#define HOMEALARMIOSIMULATORVXWORKSDRV_H

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <string>

#include "vxWorks.h"
#include "fioLib.h"
#include "taskLib.h"
#include "usrLib.h"
#include "errnoLib.h"
#include "hostLib.h"
#include "sockLib.h"
#include "socket.h"
#include "inetLib.h"
#include "in.h"

using std::string;

#define HOST_IP "192.168.0.30"
#define HOST_PORT 1314

#define RECV_BUFF_SIZE 64
#define SEND_BUFF_SIZE 64

enum DeviceType
{
    KeyType = 0,
    SwitchType = 1,
    DigitType = 2,
    StringType = 3
};

enum DeviceId
{
    // KeyType key devices
    Key0 = 0,
    Key1 = 1,
    Key2 = 2,
    Key3 = 3,
    Key4 = 4,
    Key5 = 5,
    Key6 = 6,
    Key7 = 7,
    Key8 = 8,
    Key9 = 9,
    KeyHash = 10, // #
    KeyStar = 11, // *
    KeyArm = 12,
    KeyDisarm = 13,

    // SwitchType sensor devices
    Door1 = 14,
    Door2 = 15,
    Infrared1 = 16,
    Infrared2 = 17,
    Water1 = 18,
    Water2 = 19,

    // DigitType sensor devices
    Smoke1 = 20,
    Smoke2 = 21,
    Temperature1 = 22,
    Temperature2 = 23,
    Humidity1 = 24,
    Humidity2 = 25,

    // SwitchType light device
    Light = 26,

    // SwitchType LED devices
    LEDGreen = 27,
    LEDRed = 28,

    // StringType phone devices
    PhoneNum = 29,
    PhoneType = 30,
    PhoneArea = 31,
};

enum SwitchValue
{
    Off = 0,
    On = 1,
    UnknownSwitchValue = 2
};

class CMD
{
public:
    static const string &startDelimiter();
    static void setStartDelimiter(const string &startDelimiter);

    static const string &stopDelimiter();
    static void setStopDelimiter(const string &stopDelimiter);

    CMD(int deviceType = -1, int deviceId = -1, const string &deviceValue = "");

    int deviceType() const;
    void setDeviceType(int deviceType);

    int deviceId() const;
    void setDeviceId(int deviceId);

    string deviceValue() const;
    void setDeviceValue(const string &deviceValue);

    string toCMDString() const;

private:
    static string startDelimiter_;
    static string stopDelimiter_;

    int deviceType_;
    int deviceId_;
    string deviceValue_;
};

string CMD::startDelimiter_ = ">";
string CMD::stopDelimiter_ = "<";

const string &CMD::startDelimiter()
{
    return startDelimiter_;
}

void CMD::setStartDelimiter(const string &startDelimiter)
{
    startDelimiter_ = startDelimiter;
}

const string &CMD::stopDelimiter()
{
    return stopDelimiter_;
}

void CMD::setStopDelimiter(const string &stopDelimiter)
{
    stopDelimiter_ = stopDelimiter;
}

CMD::CMD(int deviceType, int deviceId, const string &deviceValue) : deviceType_(deviceType),
                                                                    deviceId_(deviceId),
                                                                    deviceValue_(deviceValue)
{
}

int CMD::deviceType() const
{
    return this->deviceType_;
}
void CMD::setDeviceType(int deviceType)
{
    this->deviceType_ = deviceType;
}

int CMD::deviceId() const
{
    return this->deviceId_;
}
void CMD::setDeviceId(int deviceId)
{
    this->deviceId_ = deviceId;
}

string CMD::deviceValue() const
{
    return this->deviceValue_;
}
void CMD::setDeviceValue(const string &deviceValue)
{
    this->deviceValue_ = deviceValue;
}

// transform CMD to CMD string which can be transfer by socket
string CMD::toCMDString() const
{
    char cmdBuff[SEND_BUFF_SIZE];
    memset(cmdBuff, '\0', sizeof(cmdBuff));
    sprintf(cmdBuff, "%s%d %d %s%s",
            CMD::startDelimiter_.c_str(),
            this->deviceType_,
            this->deviceId_,
            this->deviceValue_.c_str(),
            CMD::stopDelimiter_.c_str());
    return cmdBuff;
}

class HomeAlarmIOBase
{
public:
    HomeAlarmIOBase(const string &hostIP = HOST_IP, int hostPort = HOST_PORT);
    virtual ~HomeAlarmIOBase();

    string hostIP() const;
    void setHostIP(const string &hostIP);

    int hostPort() const;
    void setHostPort(int hostPort);

    int clientSock() const;
    void setClientSock(int clientSock);

    // start function
    void startIOSimulator();

    // callback functions.
    // can be override in derived classes.
    virtual void onArm();
    virtual void onDisarm();
    virtual void onKey(int keyId);
    virtual void onSwitchSensor(int sensorId, int switchValue);
    virtual void onDigitSensor(int sensorId, double digitValue);

    // set functions.
    STATUS setLight(int LightId, int switchValue);
    STATUS setLED(int LEDId, int switchValue);
    STATUS setCall(const string &callNum, const string &callType, const string &callArea);

private:
    STATUS sendCMD(const string &cmdString);
    void recvCMD();
    STATUS processCMD(const string &cmdString);

    int str2int(const string &str, int start, int stop);

    string hostIP_;
    int hostPort_;
    int clientSock_; // socket opened to server
};

HomeAlarmIOBase::HomeAlarmIOBase(const string &hostIP, int hostPort) : hostIP_(hostIP),
                                                                       hostPort_(hostPort),
                                                                       clientSock_(ERROR)
{
}

HomeAlarmIOBase::~HomeAlarmIOBase()
{
    if (this->clientSock_ != ERROR)
        close(this->clientSock_);
}

string HomeAlarmIOBase::hostIP() const
{
    return this->hostIP_;
}
void HomeAlarmIOBase::setHostIP(const string &hostIP)
{
    this->hostIP_ = hostIP;
}

int HomeAlarmIOBase::hostPort() const
{
    return this->hostPort_;
}
void HomeAlarmIOBase::setHostPort(int hostPort)
{
    this->hostPort_ = hostPort;
}

int HomeAlarmIOBase::clientSock() const
{
    return this->clientSock_;
}
void HomeAlarmIOBase::setClientSock(int clientSock)
{
    this->clientSock_ = clientSock;
}

void HomeAlarmIOBase::onArm()
{
    // printf("onArm\n");
}
void HomeAlarmIOBase::onDisarm()
{
    // printf("onDisarm\n");
}
void HomeAlarmIOBase::onKey(int keyId)
{
    // printf("onKey(%d)\n", keyId);
}
void HomeAlarmIOBase::onSwitchSensor(int sensorId, int switchValue)
{
    // printf("onSwitchSensor(%d, %d)\n", sensorId, switchValue);
}
void HomeAlarmIOBase::onDigitSensor(int sensorId, double digitValue)
{
    // printf("onDigitSensor(%d, %f)\n", sensorId, digitValue);
}

STATUS HomeAlarmIOBase::setLight(int lightId, int switchValue)
{
    CMD cmd(SwitchType, lightId);
    if (switchValue == Off)
        cmd.setDeviceValue("0");
    else if (switchValue == On)
        cmd.setDeviceValue("1");
    else
        cmd.setDeviceValue("2");
    return this->sendCMD(cmd.toCMDString());
}

STATUS HomeAlarmIOBase::setLED(int LEDId, int switchValue)
{
    CMD cmd(SwitchType, LEDId);
    if (switchValue == Off)
        cmd.setDeviceValue("0");
    else if (switchValue == On)
        cmd.setDeviceValue("1");
    else
        cmd.setDeviceValue("2");
    return this->sendCMD(cmd.toCMDString());
}

STATUS HomeAlarmIOBase::setCall(const string &phoneNum, const string &phoneType, const string &phoneArea)
{
    CMD cmdNum(StringType, PhoneNum, phoneNum);
    if (this->sendCMD(cmdNum.toCMDString()) == ERROR)
        return ERROR;
    CMD cmdType(StringType, PhoneType, phoneType);
    if (this->sendCMD(cmdType.toCMDString()) == ERROR)
        return ERROR;
    CMD cmdArea(StringType, PhoneArea, phoneArea);
    return this->sendCMD(cmdArea.toCMDString());
}

// send CMD string to the simulator
STATUS HomeAlarmIOBase::sendCMD(const string &cmdString)
{
    if (this->clientSock_ == ERROR)
        return ERROR;

    int sendBytesNum;
    if ((sendBytesNum = send(this->clientSock_, cmdString.c_str(), cmdString.size(), 0)) != ERROR)
    {
        // printf("send %s successed\n", cmdString.c_str());
        return OK;
    }

    return ERROR;
}

int HomeAlarmIOBase::str2int(const string &str, int start, int stop)
{
    int res = 0;
    for (int i = start; i <= stop; ++i)
    {
        res = res * 10 + (str[i] - '0');
    }
    return res;
}

STATUS HomeAlarmIOBase::processCMD(const string &cmdString)
{
    int len = cmdString.size();
    if (len < 4)
        return ERROR;

    int deviceType;
    int deviceId;

    // extract device type
    int start = 0;
    int i = 0;
    while (cmdString[i] != ' ' && i < len)
        ++i;
    if (i >= len)
        return ERROR;
    deviceType = this->str2int(cmdString, start, i - 1);
    ++i;
    if (i >= len)
        return ERROR;

    // extract device id
    start = i;
    while (cmdString[i] != ' ' && i < len)
        ++i;
    if (i >= len)
        return ERROR;
    deviceId = this->str2int(cmdString, start, i - 1);
    ++i;

    // extract device value
    start = i;

    // call the callback functions according to device type
    switch (deviceType)
    {
    case KeyType:
        if (deviceId == KeyArm)
            this->onArm();
        else if (deviceId == KeyDisarm)
            this->onDisarm();
        else
            this->onKey(deviceId);
        break;
    case SwitchType:
        int switchValue;
        sscanf(cmdString.c_str() + start, "%d", &switchValue);
        this->onSwitchSensor(deviceId, switchValue);
        break;
    case DigitType:
        double digitValue;
        sscanf(cmdString.c_str() + start, "%lf", &digitValue);
        this->onDigitSensor(deviceId, digitValue);
        break;
    default:
        return ERROR;
        break;
    }
    return OK;
}

void HomeAlarmIOBase::recvCMD()
{
    if (this->clientSock_ == ERROR)
        return;

    char recvBuff[RECV_BUFF_SIZE];
    int recvBytesNum;
    string cmdBuff;
    string cmdString;
    while (true)
    {
        memset(recvBuff, '\0', sizeof(recvBuff));
        // recv CMD string from host
        if ((recvBytesNum = recv(this->clientSock_, recvBuff, sizeof(recvBuff) - 1, 0)) != ERROR)
        {
            cmdBuff.append(recvBuff);

            if (cmdBuff.empty() || cmdBuff.size() < 6)
            {
                cmdBuff = "";
                continue;
            }

            int start, stop;
            int startDelimiterSize = CMD::startDelimiter().size();
            int stopDelimiterSize = CMD::stopDelimiter().size();
            while (true) // extract all CMD strings
            {
                if ((start = cmdBuff.find(CMD::startDelimiter())) == -1)
                {
                    cmdBuff = "";
                    break;
                }
                if ((stop = cmdBuff.find(CMD::stopDelimiter(), start + startDelimiterSize)) == -1)
                {
                    cmdBuff = cmdBuff.substr(start);
                    break;
                }

                // extract one actual CMD string between CMD start delimiter and CMD stop delimiter
                cmdString = cmdBuff.substr(start + startDelimiterSize, stop - start - startDelimiterSize);
                if (this->processCMD(cmdString) == ERROR)
                {
                    printf("Invalid CMD string: %s", cmdString.c_str());
                }

                cmdBuff = cmdBuff.substr(stop + stopDelimiterSize);
            }
        }
    }
}

void HomeAlarmIOBase::startIOSimulator()
{
    struct sockaddr_in serverAddr; // server's address

    // Zero out the sock_addr structures.
    // This MUST be done before the socket call.
    bzero((char *)&serverAddr, sizeof(serverAddr));

    // Open the socket.
    // Use ARPA Internet address format and stream sockets.
    // Format described in "socket.h".
    int clientSock = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSock == ERROR)
    {
        printf("Socket failed to open\n");
        return;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(this->hostPort_);
    // get server's Internet address
    if ((serverAddr.sin_addr.s_addr = inet_addr(const_cast<char *>(this->hostIP_.c_str()))) == ERROR &&
        (serverAddr.sin_addr.s_addr = hostGetByName(const_cast<char *>(this->hostIP_.c_str()))) == ERROR)
    {
        printf("Invalid host: \"%s\"\n", this->hostIP_.c_str());
        close(clientSock);
        return;
    }
    printf("Server's address is %x\n", htonl(serverAddr.sin_addr.s_addr));

    // connect to host
    if (connect(clientSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == ERROR)
    {
        printf("Connection failed\n");
        close(clientSock);
        return;
    }
    printf("Connected\n");

    this->clientSock_ = clientSock;
    // taskSpawn("tCMDReceiver", 100, VX_FP_TASK, 20000, FUNCPTR(recvCMD), (int)this, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    this->recvCMD();
}

// STATUS main()
// {
//     HomeAlarmIOBase app();
//     // call this function will block this task.
//     // because there is a loop for listening CMD from host.
//     startIOSimulator(&app);

//     return OK;
// }

#endif