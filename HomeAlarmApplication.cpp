#include "HomeAlarmIOSimulatorVxWorksDrv.h"

class HomeAlarmApplication : public HomeAlarmIOBase
{
public:
    HomeAlarmApplication();
    HomeAlarmApplication(const string &hostIP, int hostPort);
    ~HomeAlarmApplication();

    // override default callback functions
    void onArm();
    void onDisarm();
    void onKey(int keyId);
    void onSwitchSensor(int sensorId, int switchValue);
    void onDigitSensor(int sensorId, double digitValue);

private:
};

HomeAlarmApplication::HomeAlarmApplication() : HomeAlarmIOBase()
{
}

HomeAlarmApplication::HomeAlarmApplication(const string &hostIP, int hostPort) : HomeAlarmIOBase(hostIP, hostPort)
{
}

HomeAlarmApplication::~HomeAlarmApplication()
{
}

void HomeAlarmApplication::onArm()
{
    printf("onArm\n");
}
void HomeAlarmApplication::onDisarm()
{
    printf("onDisarm\n");
}
void HomeAlarmApplication::onKey(int keyId)
{
    printf("onKey(%d)\n", keyId);
    switch (keyId)
    {
    case Key0:
        this->setLight(Light, On);
        break;
    case Key1:
        this->setLight(Light, Off);
        break;
    case Key2:
        this->setLED(LEDGreen, On);
        break;
    case Key3:
        this->setLED(LEDGreen, Off);
        break;
    case Key4:
        this->setLED(LEDRed, On);
        break;
    case Key5:
        this->setLED(LEDRed, Off);
        break;
    case KeyHash:
        this->setCall("110", "police", "cn");
        break;
    case KeyStar:
        this->setCall("", "", "");
        break;
    default:
        break;
    }
}
void HomeAlarmApplication::onSwitchSensor(int sensorId, int switchValue)
{
    printf("onSwitchSensor(%d, %d)\n", sensorId, switchValue);
}
void HomeAlarmApplication::onDigitSensor(int sensorId, double digitValue)
{
    printf("onDigitSensor(%d, %f)\n", sensorId, digitValue);
}

STATUS main()
{
    HomeAlarmApplication app;
    // call this function will block this task.
    // because there is a loop for listening CMD from host.
    app.startIOSimulator();

    return OK;
}
