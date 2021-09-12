#include <WINDOWS.H>
#include <stdint.h>
#include <STDIO.H>

#include "res.h"

#include "lusb0_usb.h"

#pragma comment(lib, "libusb.lib")

BOOL WINAPI DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd )
{
	
	DialogBoxParam(hInstance, (char*)IDD_DIALOG1, NULL, DlgProc, 0);
	return 0;
}

usb_dev_handle *open_dev(void);

usb_dev_handle *open_dev(void)
{
    struct usb_bus *bus;
    struct usb_device *dev;
	
    for (bus = usb_get_busses(); bus; bus = bus->next)
    {
        for (dev = bus->devices; dev; dev = dev->next)
        {
            if (dev->descriptor.idVendor == 0x1234
				&& dev->descriptor.idProduct == 0x5678)
            {
                return usb_open(dev);
            }
        }
    }
    return NULL;
}

usb_dev_handle *dev = NULL;

char tmp[64];
uint8_t i;

BOOL WINAPI DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_CLOSE:
		EndDialog(hWnd, 0);
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_LED_ON:
			usb_init(); /* initialize the library */
			usb_find_busses(); /* find all busses */
			usb_find_devices(); /* find all connected devices */
			dev = open_dev();
			if (!dev) {
				MessageBox(hWnd, "Khong tim thay thiet bi USB", NULL, MB_OK);
				return 0;
			}
			if (usb_set_configuration(dev, 0x01) < 0) {
				usb_close(dev);
				MessageBox(hWnd, "Khong set duoc config", NULL, MB_OK);
				return 0;
			}
			if (usb_claim_interface(dev, 0) < 0) {
				usb_close(dev);
				MessageBox(hWnd, "Khong set duoc interface", NULL, MB_OK);
				return 0;
			}
			tmp[0] = 0x01;
			for (i = 1; i < 64; ++i) {
				tmp[i] = 0x69;
			}
			if (usb_bulk_write(dev, 0x01, tmp, sizeof(tmp), 5000) < 0) {
				usb_close(dev);
				MessageBox(hWnd, "Khong gui duoc du lieu", NULL, MB_OK);
				return 0;
			}
			usb_release_interface(dev, 0);
			usb_close(dev);
			MessageBox(hWnd, "Gui du lieu thanh cong", NULL, MB_OK);
			break;
		case IDC_LED_OFF:
			usb_init(); /* initialize the library */
			usb_find_busses(); /* find all busses */
			usb_find_devices(); /* find all connected devices */
			dev = open_dev();
			if (!dev) {
				MessageBox(hWnd, "Khong tim thay thiet bi USB", NULL, MB_OK);
				return 0;
			}
			if (usb_set_configuration(dev, 0x01) < 0) {
				usb_close(dev);
				MessageBox(hWnd, "Khong set duoc config", NULL, MB_OK);
				return 0;
			}
			if (usb_claim_interface(dev, 0) < 0) {
				usb_close(dev);
				MessageBox(hWnd, "Khong set duoc interface", NULL, MB_OK);
				return 0;
			}
			tmp[0] = 0x02;
			for (i = 1; i < 64; ++i) {
				tmp[i] = 0x71;
			}
			if (usb_bulk_write(dev, 0x01, tmp, sizeof(tmp), 5000) < 0) {
				usb_close(dev);
				MessageBox(hWnd, "Khong gui duoc du lieu", NULL, MB_OK);
				return 0;
			}
			usb_release_interface(dev, 0);
			usb_close(dev);
			MessageBox(hWnd, "Gui du lieu thanh cong", NULL, MB_OK);
			break;
		case IDC_SEND_READ:
			usb_init(); /* initialize the library */
			usb_find_busses(); /* find all busses */
			usb_find_devices(); /* find all connected devices */
			dev = open_dev();
			if (!dev) {
				MessageBox(hWnd, "Khong tim thay thiet bi USB", NULL, MB_OK);
				return 0;
			}
			if (usb_set_configuration(dev, 0x01) < 0) {
				usb_close(dev);
				MessageBox(hWnd, "Khong set duoc config", NULL, MB_OK);
				return 0;
			}
			if (usb_claim_interface(dev, 0) < 0) {
				usb_close(dev);
				MessageBox(hWnd, "Khong set duoc interface", NULL, MB_OK);
				return 0;
			}
			tmp[0] = 0x03;
			for (i = 1; i < 64; ++i) {
				tmp[i] = (char)0x81;
			}
			if (usb_bulk_write(dev, 0x01, tmp, sizeof(tmp), 5000) < 0) {
				usb_close(dev);
				MessageBox(hWnd, "Khong gui duoc du lieu", NULL, MB_OK);
				return 0;
			}
			if (usb_bulk_read(dev, 0x82, tmp, sizeof(tmp), 5000) < 0) {
				usb_close(dev);
				MessageBox(hWnd, "Khong doc duoc du lieu", NULL, MB_OK);
				return 0;
			}

			char szTg[100];
			sprintf(szTg, "%02X %02X %02X %02X", (uint8_t)tmp[0], (uint8_t)tmp[1], (uint8_t)tmp[2], (uint8_t)tmp[3]);
			MessageBox(hWnd, szTg, "Du lieu nhan tu USB", MB_OK);
			usb_release_interface(dev, 0);
			usb_close(dev);
			MessageBox(hWnd, "Gui du lieu thanh cong", NULL, MB_OK);
			break;
		}
		break;
	}
	return 0;
}
