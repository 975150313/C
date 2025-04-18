#include "gui/MainWidget.h"
#include <QApplication>

int main1(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWidget w;
    w.show();
    return  a.exec();
}


#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>

#include <./hidapi.h>
#include <windows.h>
#include <./hidapi_winapi.h>


const char *hid_bus_name(hid_bus_type bus_type) {
    static const char *const HidBusTypeName[] = {
        "Unknown",
        "USB",
        "Bluetooth",
        "I2C",
        "SPI",
    };

    if ((int)bus_type < 0)
        bus_type = HID_API_BUS_UNKNOWN;
    if ((int)bus_type >= (int)(sizeof(HidBusTypeName) / sizeof(HidBusTypeName[0])))
        bus_type = HID_API_BUS_UNKNOWN;

    return HidBusTypeName[bus_type];
}

void print_device(struct hid_device_info *cur_dev) {
    printf("Device Found\n  type: 0x%04hX 0x%04hX\n  path: %s\n  serial_number: %ls", cur_dev->vendor_id, cur_dev->product_id, cur_dev->path, cur_dev->serial_number);
    printf("\n");
    printf("  Manufacturer: %ls\n", cur_dev->manufacturer_string);
    printf("  Product:      %ls\n", cur_dev->product_string);
    printf("  Release:      %hx\n", cur_dev->release_number);
    printf("  Interface:    %d\n",  cur_dev->interface_number);
    printf("  Usage (page): 0x%hx (0x%hx)\n", cur_dev->usage, cur_dev->usage_page);
    printf("  Bus type: %u (%s)\n", (unsigned)cur_dev->bus_type, hid_bus_name(cur_dev->bus_type));
    printf("\n");
}

void print_hid_report_descriptor_from_device(hid_device *device) {
    unsigned char descriptor[HID_API_MAX_REPORT_DESCRIPTOR_SIZE];
    int res = 0;

    printf("  Report Descriptor: ");
    res = hid_get_report_descriptor(device, descriptor, sizeof(descriptor));
    if (res < 0) {
        printf("error getting: %ls", hid_error(device));
    }
    else {
        printf("(%d bytes)", res);
    }

//    for (int i = 0; i < res; i++) {
//        if (i % 10 == 0) {
//            printf("\n");
//        }
//        printf("0x%02x, ", descriptor[i]);
//    }
//    printf("\n");
}

void print_hid_report_descriptor_from_path(const char *path) {
    hid_device *device = hid_open_path(path);
    if (device) {
        print_hid_report_descriptor_from_device(device);
        hid_close(device);
    }
    else {
        printf("  Report Descriptor: Unable to open device by path\n");
    }
}

void print_devices(struct hid_device_info *cur_dev) {
    for (; cur_dev; cur_dev = cur_dev->next) {
        print_device(cur_dev);
    }
}

void print_devices_with_descriptor(struct hid_device_info *cur_dev) {
    for (; cur_dev; cur_dev = cur_dev->next) {
        print_device(cur_dev);
        print_hid_report_descriptor_from_path(cur_dev->path);
    }
}

int main(int argc, char* argv[])
{
    unsigned char buf[256];
    const int MAX_STR = 255;
    wchar_t wstr[MAX_STR];
    hid_device *handle;
    int i;
    int res;
    struct hid_device_info *devs;

    if (hid_init())
        return -1;

    devs = hid_enumerate(0x0, 0x0);
    print_devices_with_descriptor(devs);
    hid_free_enumeration(devs);
    return 1;

    // Set up the command buffer.
    memset(buf,0x00,sizeof(buf));
    buf[0] = 0x01;
    buf[1] = 0x81;


    // Open the device using the VID, PID,
    ////handle = hid_open(0x4d8, 0x3f, L"12345");
    handle = hid_open(0x256F, 0xC63A, NULL);
    if (!handle) {
        printf("unable to open device\n");
        hid_exit();
        return 1;
    }

    hid_winapi_set_write_timeout(handle, 5000);

    // Read the Manufacturer String
    wstr[0] = 0x0000;
    res = hid_get_manufacturer_string(handle, wstr, MAX_STR);
    if (res < 0)
        printf("Unable to read manufacturer string\n");
    printf("Manufacturer String: %ls\n", wstr);

    // Read the Product String
    wstr[0] = 0x0000;
    res = hid_get_product_string(handle, wstr, MAX_STR);
    if (res < 0)
        printf("Unable to read product string\n");
    printf("Product String: %ls\n", wstr);

    // Read the Serial Number String
    wstr[0] = 0x0000;
    res = hid_get_serial_number_string(handle, wstr, MAX_STR);
    if (res < 0)
        printf("Unable to read serial number string\n");
    printf("Serial Number String: (%d) %ls\n", wstr[0], wstr);

    print_hid_report_descriptor_from_device(handle);

    struct hid_device_info* info = hid_get_device_info(handle);
    if (info == NULL) {
        printf("Unable to get device info\n");
    } else {
        print_devices(info);
    }

    // Read Indexed String 1
    wstr[0] = 0x0000;
    res = hid_get_indexed_string(handle, 1, wstr, MAX_STR);
    if (res < 0)
        printf("Unable to read indexed string 1\n");
    printf("Indexed String 1: %ls\n", wstr);
    hid_set_nonblocking(handle, 1);

    hid_close(handle);
    hid_exit();
    return 0;
}
