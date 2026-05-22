#!/usr/bin/python3 -u
# this requires cython-hidapi:
# pip3 install hidapi

import argparse
import hid
import time

VID = 0x1FC9
PID = 0x00A2


def print_device_info(device):
    print("")
    print("Manufacturer: %s" % device.get_manufacturer_string())
    print("Product: %s" % device.get_product_string())
    print("Serial No: %s" % device.get_serial_number_string())
    print("")


def decode_switch_report(data):
    if len(data) < 3:
        return None

    if data[0] != 0xA5:
        return None

    sw = data[1]
    state = data[2]

    if state != 1:
        return None

    if sw == 2:
        return "SW2 pressed"
    if sw == 3:
        return "SW3 pressed"

    return f"Unknown switch ({sw}) pressed"


def poll_switch_reports(device):
    data = device.read(8)
    if data:
        message = decode_switch_report(data)
        if message is not None:
            print(message)


def send_led_report(device, led, state):
    # write() includes report ID as first byte.
    # report payload: 0xFF, led, state, padding (8-byte payload total)
    device.write([0x00, 0xFF, led, state] + [0] * 5)


def run_sender(device):
    for led in range(2):
        print("Blink red LED" if led == 0 else "Blink green LED")
        for _ in range(10):
            send_led_report(device, led, 1)
            poll_switch_reports(device)
            time.sleep(0.1)

            send_led_report(device, led, 0)
            poll_switch_reports(device)
            time.sleep(0.1)


def run_receiver(device):
    print("Listening for switch events... Press CTRL-C to quit")
    while True:
        poll_switch_reports(device)
        time.sleep(0.01)


def parse_args():
    parser = argparse.ArgumentParser(description="USB HID sender/receiver")
    parser.add_argument(
        "--mode",
        choices=["send", "recv", "both"],
        default="both",
        help="send LED commands, receive switch events, or both",
    )
    return parser.parse_args()


def main():
    args = parse_args()
    device = None

    try:
        device = hid.device()
        device.open(VID, PID)
        device.set_nonblocking(1)

        print_device_info(device)

        if args.mode in ("send", "both"):
            run_sender(device)

        if args.mode in ("recv", "both"):
            run_receiver(device)

    except IOError:
        print("Unable to open HID device (is firmware running and USB (J8) connected?)")

    except KeyboardInterrupt:
        print("CTRL-C: Quit")

    finally:
        if device is not None:
            device.close()

    print("Done")


if __name__ == "__main__":
    main()
