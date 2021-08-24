#include "esphome.h"

using namespace esphome;
using namespace remote_receiver;
using namespace remote_base;
using namespace sensor;

static const char *ALECTO_TAG = "alecto";

class AlectoComponent : public Component, public remote_base::RemoteReceiverListener, public remote_base::RawDumper
{

private:
  RemoteReceiverComponent *receiver;
  int tempId;
  int rainId;

public:
  AlectoComponent(RemoteReceiverComponent *_receiver, int _tempId, int _rainId) : receiver(_receiver), tempId(_tempId), rainId(_rainId) {}

  Sensor tempSensor;
  Sensor humiditySensor;
  Sensor rainSensor;
  Sensor windGustSensor;
  Sensor windAvgSensor;
  Sensor windDirSensor;
  Sensor rainBatterySensor;
  Sensor tempBatterySensor;

  bool on_receive(RemoteReceiveData src) override
  {
    if (src.size() < 72)
      return false;

    uint8_t n[9];
    for (int i = 0; i < 9; i++)
      n[i] = 0;
    for (int i = 0; i < src.size() && i < 72; i += 2)
    {
      if (src.expect_item(500, 2000))
      {
        //0
        //ESP_LOGD(ALECTO_TAG, "i/8: %d",i/8);
        n[i / 8] >>= 1;
      }
      else if (src.expect_item(500, 4000))
      {
        //1
        //ESP_LOGD(ALECTO_TAG, "i/8: %d",i/8);
        n[i / 8] >>= 1;
        n[i / 8] |= 8;
      }
      else
      {
        //ESP_LOGD(ALECTO_TAG, "error decoding at %d, size: %d", i, src.size());
        return false;
      }
    }

    int type = 0;
    if ((n[2] & 0x6) != 0x6)
    {
      type = 0x1;
    }
    else if (n[3] == 0x1)
    {
      type = 0x2;
    }
    else if ((n[3] & 0x7) == 0x7)
    {
      type = 0x3;
    }
    else if (n[3] == 0x3)
    {
      type = 0x4;
    }
    else
    {
      return false;
    }

    int checksum = 0;
    if (type == 0x4)
      checksum = (0x7 + n[0] + n[1] + n[2] + n[3] + n[4] + n[5] + n[6] + n[7]) & 0xf;
    else
      checksum = (0xf - n[0] - n[1] - n[2] - n[3] - n[4] - n[5] - n[6] - n[7]) & 0xf;

    if (n[8] != checksum)
    {
      ESP_LOGD(ALECTO_TAG, "checksum calculated: %d, checksum packet: %d", checksum, n[8]);
      return false;
    }

    //dump(src);
    // ESP_LOGD(ALECTO_TAG, "packet size: %d", src.size());
    // for(int i=0;i<9;i++) {
    //   ESP_LOGD(ALECTO_TAG, "nimble %d = %d", i, out[i]);
    // }

    int id = n[0] + (n[1] << 4);
    boolean battery = !(n[2] & 0b00001000);
    ESP_LOGD(ALECTO_TAG, "id: %x, battery: %d", id, battery);
    int16_t temperature = 0;
    float windavg = 0;
    int humidity = 0;
    int winddir = 0;
    float windgust = 0;
    float rain = 0;

    switch (type)
    {
    case 1:
      if (id != tempId)
        return false;
      temperature = (n[3] << 4) | (n[4] << 8) | (n[5] << 12);
      temperature /= 16;
      if(temperature > 700 || temperature < -400) // check temperature range
        return false;
      humidity = n[7] * 10 + n[6];
      ESP_LOGD(ALECTO_TAG, "id: %x, temperature: %d, humidity: %d, battery: %d", id, temperature, humidity, battery);
      tempSensor.publish_state(((float)temperature) / 10);
      humiditySensor.publish_state(humidity);
      tempBatterySensor.publish_state(battery * 100);
      break;
    case 2:
      if (id != tempId)
        return false;
      windavg = (n[6] + n[7] * 16) * 0.2;
      windAvgSensor.publish_state(windavg);
      tempBatterySensor.publish_state(battery * 100);
      ESP_LOGD(ALECTO_TAG, "id: %x, windavg: %f, battery:%d", id, windavg, battery);
      break;
    case 3:
      if (id != tempId)
        return false;
      winddir = ((n[3] >> 3) & 1) + n[4] * 2 + n[5] * 32;
      windgust = (n[6] + n[7] * 16) * 0.2;
      windDirSensor.publish_state(winddir);
      windGustSensor.publish_state(windgust);
      tempBatterySensor.publish_state(battery * 100);
      ESP_LOGD(ALECTO_TAG, "id: %x, winddir: %d, windgust: %f, battery:%d", id, winddir, windgust, battery);
      break;
    case 4:
      if (id != rainId)
        return false;
      rain = (n[4] + n[5] * 16 + n[6] * 256 + n[7] * 4096) * 0.25;
      ESP_LOGD(ALECTO_TAG, "id: %x, rain: %f, battery:%d", id, rain, battery);
      rainSensor.publish_state(rain);
      rainBatterySensor.publish_state(battery * 100);
      break;
    }

    return false;
  }

  void setup()
  {
    receiver->register_listener(this);
  }

  void loop()
  {
  }
};
