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

#define HOST_IP "192.168.0.10"
#define HOST_PORT 1314

#define RECV_BUFF_SIZE 64

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

class HomeAlarmIOBase
{
public:
    HomeAlarmIOBase(const string &hostIP, int hostPort = HOST_PORT);
    virtual ~HomeAlarmIOBase();

    string hostIP() const;
    void setHostIP(const string &hostIP);

    int hostPort() const;
    void setHostPort(int hostPort);

    int clientSock() const;
    void setClientSock(int clientSock);

    // callback functions.
    // can be override in derived classes.
    virtual void onArm();
    virtual void onDisarm();
    virtual void onKey(int keyId);
    virtual void onSwitchSensor(int sensorId, int switchValue);
    virtual void onDigitSensor(int sensorId, double digitValue);

private:
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
    printf("onArm\n");
}
void HomeAlarmIOBase::onDisarm()
{
    printf("onDisarm\n");
}
void HomeAlarmIOBase::onKey(int keyId)
{
    printf("onKey(%d)\n", keyId);
}
void HomeAlarmIOBase::onSwitchSensor(int sensorId, int switchValue)
{
    printf("onSwitchSensor(%d, %d)\n", sensorId, switchValue);
}
void HomeAlarmIOBase::onDigitSensor(int sensorId, double digitValue)
{
    printf("onDigitSensor(%d, %f)\n", sensorId, digitValue);
}

STATUS processCMD(const string &cmdString, HomeAlarmIOBase *homeAlarmIO)
{
    int len = cmdString.size();
    if (len < 4)
        return ERROR;

    int deviceType;
    int deviceId;
    string deviceValue;

    // extract device type
    int start = 0;
    int i = 0;
    while (cmdString[i] != ' ' && i < len)
        ++i;
    if (i >= len)
        return ERROR;
    sscanf(cmdString.substr(start, i - start).c_str(), "%d", &deviceType);
    ++i;
    if (i >= len)
        return ERROR;

    // extract device id
    start = i;
    while (cmdString[i] != ' ' && i < len)
        ++i;
    if (i >= len)
        return ERROR;
    sscanf(cmdString.substr(start, i - start).c_str(), "%d", &deviceId);
    ++i;

    // extract device value
    start = i;
    deviceValue = cmdString.substr(start);

    // call the callback functions according to device type
    switch (deviceType)
    {
    case KeyType:
        if (deviceId == KeyArm)
            homeAlarmIO->onArm();
        else if (deviceId == KeyDisarm)
            homeAlarmIO->onDisarm();
        else
            homeAlarmIO->onKey(deviceId);
        break;
    case SwitchType:
        int switchValue;
        sscanf(deviceValue.c_str(), "%d", &switchValue);
        homeAlarmIO->onSwitchSensor(deviceId, switchValue);
        break;
    case DigitType:
        double digitValue;
        sscanf(deviceValue.c_str(), "%lf", &digitValue);
        homeAlarmIO->onDigitSensor(deviceId, digitValue);
        break;
    default:
        return ERROR;
        break;
    }
    return OK;
}

STATUS recvCMD(HomeAlarmIOBase *homeAlarmIO)
{
    if (homeAlarmIO->clientSock() == ERROR)
        return ERROR;

    char recvBuff[RECV_BUFF_SIZE];
    int recvBytesNum;
    string cmdBuff;
    while (true)
    {
        memset(recvBuff, '\0', sizeof(recvBuff));
        // recv CMD string from host
        if ((recvBytesNum = recv(homeAlarmIO->clientSock(), recvBuff, sizeof(recvBuff) - 1, 0)) != ERROR)
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
                processCMD(cmdBuff.substr(start + startDelimiterSize, stop - start - startDelimiterSize), homeAlarmIO);

                cmdBuff = cmdBuff.substr(stop + stopDelimiterSize);
            }
        }
    }
}

STATUS startIOSimulator(HomeAlarmIOBase *homeAlarmIO)
{
    struct sockaddr_in serverAddr; // server's address
    struct sockaddr_in clientAddr; // client's address

    // Zero out the sock_addr structures.
    // This MUST be done before the socket call.
    bzero((char *)&serverAddr, sizeof(serverAddr));
    bzero((char *)&clientAddr, sizeof(clientAddr));

    // Open the socket.
    // Use ARPA Internet address format and stream sockets.
    // Format described in "socket.h".
    int clientSock = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSock == ERROR)
    {
        printf("Socket failed to open\n");
        return ERROR;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(homeAlarmIO->hostPort());
    // get server's Internet address
    if ((serverAddr.sin_addr.s_addr = inet_addr(const_cast<char *>(homeAlarmIO->hostIP().c_str()))) == ERROR &&
        (serverAddr.sin_addr.s_addr = hostGetByName(const_cast<char *>(homeAlarmIO->hostIP().c_str()))) == ERROR)
    {
        printf("Invalid host: \"%s\"\n", homeAlarmIO->hostIP().c_str());
        close(clientSock);
        return ERROR;
    }
    printf("Server's address is %x:\n", htonl(serverAddr.sin_addr.s_addr));

    // connect to host
    if (connect(clientSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == ERROR)
    {
        printf("Connection failed\n");
        close(clientSock);
        return ERROR;
    }
    printf("Connected\n");

    homeAlarmIO->setClientSock(clientSock);
    // taskSpawn("tCMDReceiver", 100, VX_FP_TASK, 20000, FUNCPTR(recvCMD), (int)homeAlarmIO, 0, 0, 0, 0, 0, 0, 0, 0, 0);
    recvCMD(homeAlarmIO);
}

STATUS main()
{
    HomeAlarmIOBase app(HOST_IP);
    // call this function will block this task.
    // because there is a loop for listening CMD from host.
    startIOSimulator(&app);

    return OK;
}