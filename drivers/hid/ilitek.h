#ifndef USB_VENDOR_ID_ILITEK
	#define USB_VENDOR_ID_ILITEK	0x222A
#endif
#ifndef USB_DEVICE_ID_ILITEK_MT
	#define USB_DEVICE_ID_ILITEK_MT	0x0088
#endif
#define TOUCH_PROTOCOL_B
//#define SET_KEY
#ifdef SET_KEY
	#define KEY_COUNT 3
	#define KEY_XLEN 256
	#define KEY_YLEN 256
	int key_id[] = {1,2,3};
	int keyinfo_x[] = {3526, 4702, 5877};
	int keyinfo_y[] = {16128, 16128, 16128};
#endif
//#define SINGLE_TOUCH
//#define ROTATE_FLAG_90
//#define ROTATE_FLAG_180
//#define ROTATE_FLAG_270