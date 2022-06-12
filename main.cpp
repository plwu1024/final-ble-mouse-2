#include "mbed.h"
#include <events/mbed_events.h>
#include "ble/BLE.h"
#include "ble/Gap.h"
#include "pretty_printer.h"
#include "BatteryService.h"
#include "DeviceInformationService.h"
#include "HIDMouseService.h"

const static char DEVICE_NAME[] = "STM32Mouse";

static events::EventQueue event_queue(/* event count */ 16 * EVENTS_EVENT_SIZE);

class MouseRunner : ble::Gap::EventHandler
{
  BLE &_ble;
  events::EventQueue &_event_queue;

//   UUID _device_information_uuid;
  DeviceInformationService _device_information_service;

//   UUID _battery_uuid;
  uint8_t _battery_level;
  BatteryService _battery_service;
  bool _is_connected;

  UUID _hid_mouse_uuid;
  HIDMouseService _hid_mouse_service;

  uint8_t _adv_buffer[ble::LEGACY_ADVERTISING_MAX_SIZE];
  ble::AdvertisingDataBuilder _adv_data_builder;

public:
  MouseRunner(BLE &ble, events::EventQueue &event_queue) : _ble(ble),
                                                           _event_queue(event_queue),
                                                        //    _device_information_uuid(GattService::UUID_DEVICE_INFORMATION_SERVICE),
                                                           _device_information_service(ble, "team2", "joystick", "0", "1.0", "1.0", "1.0"),
                                                        //    _battery_uuid(GattService::UUID_BATTERY_SERVICE),
                                                           _battery_level(100),
                                                           _battery_service(ble, _battery_level),
                                                           _hid_mouse_uuid(GattService::UUID_HUMAN_INTERFACE_DEVICE_SERVICE),
                                                           _hid_mouse_service(ble),
                                                           _is_connected(false),
                                                           _adv_data_builder(_adv_buffer)
  {
  }

  void start()
  {
    _ble.gap().setEventHandler(this);

    // call on_init_complete when ble ready.
    _ble.init(this, &MouseRunner::on_init_complete);

    // _event_queue.call_every(500, this, &MouseRunner::update_mouse);

    // this runs and never return
    _event_queue.dispatch_forever();
  }

private:
  void on_init_complete(BLE::InitializationCompleteCallbackContext *params)
  {
    if (params->error != BLE_ERROR_NONE)
    {
      print_error(params->error, "Ble initialization failed.");
      return;
    }

    print_mac_address();

    start_advertising();
  }

  void start_advertising()
  {
    /* Create advertising parameters and payload */

    ble::AdvertisingParameters adv_parameters(
        // Device connectable, scannable. no expect connection from a specific peer.
        ble::advertising_type_t::CONNECTABLE_UNDIRECTED,
        ble::adv_interval_t(ble::millisecond_t(30)));

    // TODO optionally add extra data if active scanned
    const uint8_t _vendor_specific_data[4] = {0xAD, 0xDE, 0xBE, 0xEF};
    _adv_data_builder.setManufacturerSpecificData(_vendor_specific_data);

    _ble.gap().setAdvertisingScanResponse(
        ble::LEGACY_ADVERTISING_HANDLE,
        _adv_data_builder.getAdvertisingData());

    /* now we set the advertising payload that gets sent
        during advertising without any scan requests */
    _adv_data_builder.clear();
    _adv_data_builder.setFlags();
    _adv_data_builder.setAppearance(_hid_mouse_service.appearance());
    // TODO weird!!
    _adv_data_builder.setLocalServiceList(mbed::make_Span(&_hid_mouse_uuid, 1));
    _adv_data_builder.setName(DEVICE_NAME);

    /* we add the battery level as part of the payload so it's visible to any device that scans,
     * this part of the payload will be updated periodically without affecting the rest of the payload */
    // _adv_data_builder.setServiceData(GattService::UUID_BATTERY_SERVICE, {&_battery_level, 1});

    /* Setup advertising */

    ble_error_t error = _ble.gap().setAdvertisingParameters(
        ble::LEGACY_ADVERTISING_HANDLE,
        adv_parameters);

    if (error)
    {
      print_error(error, "_ble.gap().setAdvertisingParameters() failed");
      return;
    }

    error = _ble.gap().setAdvertisingPayload(
        ble::LEGACY_ADVERTISING_HANDLE,
        _adv_data_builder.getAdvertisingData());

    if (error)
    {
      print_error(error, "_ble.gap().setAdvertisingPayload() failed");
      return;
    }

    /* Start advertising */

    error = _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);

    if (error)
    {
      print_error(error, "_ble.gap().startAdvertising() failed");
      return;
    }

    printf("BLE advertising. Please connect.\r\n");

    _event_queue.call_every(
        50ms,
        [this]()
        {
          update_mouse();
        });
    // _event_queue.call_every(
    //     1000ms,
    //     [this]() {
    //         update_battery_level();
    //     }
    // );
  }

  // void button_pressed(void) {
  //     _event_queue.call(Callback<void(bool)>(_button_service, &ButtonService::updateButtonState), true);
  // }

  // void button_released(void) {
  //     _event_queue.call(Callback<void(bool)>(_button_service, &ButtonService::updateButtonState), false);
  // }

  void update_battery_level()
  {
    // if (_battery_level-- <= 10)
    // {
    //   _battery_level = 100;
    // }

    // if (_is_connected)
    // {
    //   _battery_service.updateBatteryLevel(_battery_level);
      
    // }
    // if (ble.gap().isAdvertisingActive){
    //   /* update the payload with the new value of the bettery level, the rest of the payload remains the same */
    //   ble_error_t error = _adv_data_builder.setServiceData(GattService::UUID_BATTERY_SERVICE, make_Span(&_battery_level, 1));

    //   if (error)
    //   {
    //     print_error(error, "_adv_data_builder.setServiceData() failed");
    //     return;
    //   }
    //   /* set the new payload, we don't need to stop advertising */
    //   error = _ble.gap().setAdvertisingPayload(
    //       ble::LEGACY_ADVERTISING_HANDLE,
    //       _adv_data_builder.getAdvertisingData());

    //   if (error)
    //   {
    //     print_error(error, "_ble.gap().setAdvertisingPayload() failed");
    //     return;
    //   }
    // }
  }

  void update_mouse()
  {
    // TODO every 50ms
    // _hid_mouse_service.motion(3, 3);
    // _hid_mouse_service.button(_hid_mouse_service.BUTTON_LEFT);
    // _hid_mouse_service.SendReport();
  }

  virtual void onConnectionComplete(const ble::ConnectionCompleteEvent &event)
  {
    _is_connected = true;
    if (event.getStatus() == ble_error_t::BLE_ERROR_NONE)
    {
      printf("Client connected, you may now subscribe to updates\r\n");
    }
  }

private:
  virtual void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &)
  {
    _is_connected = false;
    printf("Connection drop. Start advertising...\r\n");
    _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
  }
};

/** Schedule processing of events from the BLE middleware in the event queue. */
void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context)
{
  event_queue.call(Callback<void()>(&context->ble, &BLE::processEvents));
}

int main()
{
  BLE &ble = BLE::Instance();
  ble.onEventsToProcess(schedule_ble_events);

  MouseRunner my_hid_mouse(ble, event_queue);
  my_hid_mouse.start();

  return 0;
}
