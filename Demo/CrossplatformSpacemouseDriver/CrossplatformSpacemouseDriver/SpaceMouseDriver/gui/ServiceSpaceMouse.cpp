#include "ServiceSpaceMouse.h"

#include <string>
#include <stdio.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <QDebug>
#include "hidapi.h"

#include "BaseSingleton.h"
#include "GlobSignal.h"
ServiceSpaceMouse::ServiceSpaceMouse()
{

}

void ServiceSpaceMouse::run()
{
    const int _3DCONNEXION_VENDOR_LOGITECH = 0x046d;			// 1133 // Logitech (3Dconnexion is made by Logitech)
    const int _3DCONNEXION_VENDOR_3DCONNECTION = 0x256F;		// 9583 // 3Dconnexion
    const int _3DCONNEXION_TRAVELER = 0xC623;				// 50723
    const int _3DCONNEXION_NAVIGATOR = 0xC626;				// 50726
    const int _3DCONNEXION_NAVIGATOR_FOR_NOTEBOOKS = 0xc628;	// 50728
    const int _3DCONNEXION_SPACEEXPLORER = 0xc627;			// 50727
    const int _3DCONNEXION_SPACEMOUSE = 0xC603;				// 50691
    const int _3DCONNEXION_SPACEMOUSEPRO = 0xC62B;			// 50731
    const int _3DCONNEXION_SPACEBALL5000 = 0xc621;			// 50721
    const int _3DCONNEXION_SPACEPILOT = 0xc625;				// 50725
    const int _3DCONNEXION_SPACEPILOTPRO = 0xc629;			// 50729
    const int _3DCONNEXION_SPACEMOUSEWIRELESSBT = 0xc63A;  // 50746  9583
    const int _3DCONNEXION_SPACEMOUSEWIRELESS = 0xC652;  // 50770

//     type: 0x256F 0xC652

    unsigned short current_vendor_id = 0x0;
    unsigned short current_product_id = 0x0;

    struct hid_device_info* devs = hid_enumerate(0x0, 0x0);
    struct hid_device_info* loop = devs;
    while (loop)
    {

        qDebug() << "vendor_id: "<< loop->vendor_id << ",  product_id: " << loop->product_id ;
        if ((loop->vendor_id == _3DCONNEXION_VENDOR_LOGITECH ||
                loop->vendor_id == _3DCONNEXION_VENDOR_3DCONNECTION)
                && (loop->product_id == _3DCONNEXION_TRAVELER ||
                    loop->product_id == _3DCONNEXION_NAVIGATOR ||
                    loop->product_id == _3DCONNEXION_NAVIGATOR_FOR_NOTEBOOKS ||
                    loop->product_id == _3DCONNEXION_SPACEEXPLORER ||
                    loop->product_id == _3DCONNEXION_SPACEMOUSE ||
                    loop->product_id == _3DCONNEXION_SPACEMOUSEPRO ||
                    loop->product_id == _3DCONNEXION_SPACEBALL5000 ||
                    loop->product_id == _3DCONNEXION_SPACEPILOT ||
                    loop->product_id == _3DCONNEXION_SPACEPILOTPRO||
                    loop->product_id == _3DCONNEXION_SPACEMOUSEWIRELESSBT||
                    loop->product_id == _3DCONNEXION_SPACEMOUSEWIRELESS))
        {
            //setup the values with the founded vendor id.s
            current_vendor_id = loop->vendor_id;
            current_product_id = loop->product_id;

            break; //break when first comaptaible device is found.
        }

        loop = loop->next;
    }
    hid_free_enumeration(devs);

    if (current_vendor_id == 0x0 || current_product_id == 0x0)
    {
        qDebug() << "SpaceMouseInfo: No compatible SpaceMouse device found!";
        return ;
    }

    //open device with current_vendor_id and current_product_id
    hid_device * m_handle = hid_open(current_vendor_id, current_product_id, NULL);
    if (!m_handle)
    {
        qDebug()  << "SpaceMouseError: Unable to open device!";
        return ;
    }

    qDebug() << "\nvendor_id: "<< loop->vendor_id << ",  product_id: " << loop->product_id ;

    const int MAX_STR = 255;
    wchar_t wstr[MAX_STR];
    wstr[0] = 0x0000;
    int res = hid_get_manufacturer_string(m_handle, wstr, MAX_STR);
    qDebug() << "Manufacturer: "<<QString::fromWCharArray(wstr);

    wstr[0] = 0x0000;
    res = hid_get_product_string(m_handle, wstr, MAX_STR);
    qDebug() << "Product: "<<QString::fromWCharArray(wstr);

    wstr[0] = 0x0000;
    res = hid_get_serial_number_string(m_handle, wstr, MAX_STR);
    qDebug() << "Serial Number: "<<QString::fromWCharArray(wstr);

    while (isRuning())
    {
        msleep(12);
        SpaceMouseStatus status;
        uint8_t buffer[32] = {0};
        int result = hid_read_timeout(m_handle, buffer, sizeof(buffer), 400); //when timeout
        if (result > 0)
        {
            status.type = buffer[0];
            uint16_t* p16 =(uint16_t*)&buffer[1];
            if(0x01 == status.type )
            {
                for(size_t i=0; i<6;++i)
                {
                    status.arr[i] = p16[i];
                }
            }
            else if(0x03 == status.type)
            {
                status.btn =  buffer[1];
            }
            Singleton(GlobSignal)->trage_event(status);
        }
    }
}


