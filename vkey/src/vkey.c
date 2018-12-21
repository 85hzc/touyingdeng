/*XiaoLiang Zou
Using keys/buttons to send user input*/

#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <termios.h>
#include <stdlib.h>             // exit
#include <sys/ioctl.h>          // ioctl
#include <string.h>             // bzero
#include "MyCom.h"

#define bzero(a, b)             memset(a, 0, b)

#define EV_SYN			0x00
#define EV_KEY			0x01
#define EV_REL			0x02
#define EV_ABS			0x03
#define EV_MSC			0x04
#define EV_SW			0x05
#define EV_LED			0x11
#define EV_SND			0x12
#define EV_REP			0x14
#define EV_FF			0x15
#define EV_PWR			0x16
#define EV_FF_STATUS	0x17
#define EV_MAX			0x1f
#define EV_CNT			(EV_MAX+1)

/*
 * Synchronization events.
 */

#define SYN_REPORT		0
#define SYN_CONFIG		1
#define SYN_MT_REPORT		2

/*
 * Keys and buttons
 *
 * Most of the keys/buttons are modeled after USB HUT 1.12
 * (see http://www.usb.org/developers/hidpage).
 * Abbreviations in the comments:
 * AC - Application Control
 * AL - Application Launch Button
 * SC - System Control
 */

#define KEY_RESERVED		0
#define KEY_ESC			1
#define KEY_1			2
#define KEY_2			3
#define KEY_3			4
#define KEY_4			5
#define KEY_5			6
#define KEY_6			7
#define KEY_7			8
#define KEY_8			9
#define KEY_9			10
#define KEY_0			11
#define KEY_MINUS		12
#define KEY_EQUAL		13
#define KEY_BACKSPACE		14
#define KEY_TAB			15
#define KEY_Q			16
#define KEY_W			17
#define KEY_E			18
#define KEY_R			19
#define KEY_T			20
#define KEY_Y			21
#define KEY_U			22
#define KEY_I			23
#define KEY_O			24
#define KEY_P			25
#define KEY_LEFTBRACE		26
#define KEY_RIGHTBRACE		27
#define KEY_ENTER		28
#define KEY_LEFTCTRL		29
#define KEY_A			30
#define KEY_S			31
#define KEY_D			32
#define KEY_F			33
#define KEY_G			34
#define KEY_H			35
#define KEY_J			36
#define KEY_K			37
#define KEY_L			38
#define KEY_SEMICOLON		39
#define KEY_APOSTROPHE		40
#define KEY_GRAVE		41
#define KEY_LEFTSHIFT		42
#define KEY_BACKSLASH		43
#define KEY_Z			44
#define KEY_X			45
#define KEY_C			46
#define KEY_V			47
#define KEY_B			48
#define KEY_N			49
#define KEY_M			50
#define KEY_COMMA		51
#define KEY_DOT			52
#define KEY_SLASH		53
#define KEY_RIGHTSHIFT		54
#define KEY_KPASTERISK		55
#define KEY_LEFTALT		56
#define KEY_SPACE		57
#define KEY_CAPSLOCK		58
#define KEY_F1			59
#define KEY_F2			60
#define KEY_F3			61
#define KEY_F4			62
#define KEY_F5			63
#define KEY_F6			64
#define KEY_F7			65
#define KEY_F8			66
#define KEY_F9			67
#define KEY_F10			68
#define KEY_NUMLOCK		69
#define KEY_SCROLLLOCK		70
#define KEY_KP7			71
#define KEY_KP8			72
#define KEY_KP9			73
#define KEY_KPMINUS		74
#define KEY_KP4			75
#define KEY_KP5			76
#define KEY_KP6			77
#define KEY_KPPLUS		78
#define KEY_KP1			79
#define KEY_KP2			80
#define KEY_KP3			81
#define KEY_KP0			82
#define KEY_KPDOT		83

#define KEY_ZENKAKUHANKAKU	85
#define KEY_102ND		86
#define KEY_F11			87
#define KEY_F12			88
#define KEY_RO			89
#define KEY_KATAKANA		90
#define KEY_HIRAGANA		91
#define KEY_HENKAN		92
#define KEY_KATAKANAHIRAGANA	93
#define KEY_MUHENKAN		94
#define KEY_KPJPCOMMA		95
#define KEY_KPENTER		96
#define KEY_RIGHTCTRL		97
#define KEY_KPSLASH		98
#define KEY_SYSRQ		99
#define KEY_RIGHTALT		100
#define KEY_LINEFEED		101
#define KEY_HOME		102
#define KEY_UP			103
#define KEY_PAGEUP		104
#define KEY_LEFT		105
#define KEY_RIGHT		106
#define KEY_END			107
#define KEY_DOWN		108
#define KEY_PAGEDOWN		109
#define KEY_INSERT		110
#define KEY_DELETE		111
#define KEY_MACRO		112
#define KEY_MUTE		113
#define KEY_VOLUMEDOWN		114
#define KEY_VOLUMEUP		115
#define KEY_POWER		116	/* SC System Power Down */
#define KEY_KPEQUAL		117
#define KEY_KPPLUSMINUS		118
#define KEY_PAUSE		119
#define KEY_SCALE		120	/* AL Compiz Scale (Expose) */

#define KEY_KPCOMMA		121
#define KEY_HANGEUL		122
#define KEY_HANGUEL		KEY_HANGEUL
#define KEY_HANJA		123
#define KEY_YEN			124
#define KEY_LEFTMETA		125
#define KEY_RIGHTMETA		126
#define KEY_COMPOSE		127

#define KEY_STOP		128	/* AC Stop */
#define KEY_AGAIN		129
#define KEY_PROPS		130	/* AC Properties */
#define KEY_UNDO		131	/* AC Undo */
#define KEY_FRONT		132
#define KEY_COPY		133	/* AC Copy */
#define KEY_OPEN		134	/* AC Open */
#define KEY_PASTE		135	/* AC Paste */
#define KEY_FIND		136	/* AC Search */
#define KEY_CUT			137	/* AC Cut */
#define KEY_HELP		138	/* AL Integrated Help Center */
#define KEY_MENU		139	/* Menu (show menu) */
#define KEY_CALC		140	/* AL Calculator */
#define KEY_SETUP		141
#define KEY_SLEEP		142	/* SC System Sleep */
#define KEY_WAKEUP		143	/* System Wake Up */
#define KEY_FILE		144	/* AL Local Machine Browser */
#define KEY_SENDFILE		145
#define KEY_DELETEFILE		146
#define KEY_XFER		147
#define KEY_PROG1		148
#define KEY_PROG2		149
#define KEY_WWW			150	/* AL Internet Browser */
#define KEY_MSDOS		151
#define KEY_COFFEE		152	/* AL Terminal Lock/Screensaver */
#define KEY_SCREENLOCK		KEY_COFFEE
#define KEY_DIRECTION		153
#define KEY_CYCLEWINDOWS	154
#define KEY_MAIL		155
#define KEY_BOOKMARKS		156	/* AC Bookmarks */
#define KEY_COMPUTER		157
#define KEY_BACK		158	/* AC Back */
#define KEY_FORWARD		159	/* AC Forward */
#define KEY_CLOSECD		160
#define KEY_EJECTCD		161
#define KEY_EJECTCLOSECD	162
#define KEY_NEXTSONG		163
#define KEY_PLAYPAUSE		164
#define KEY_PREVIOUSSONG	165
#define KEY_STOPCD		166
#define KEY_RECORD		167
#define KEY_REWIND		168
#define KEY_PHONE		169	/* Media Select Telephone */
#define KEY_ISO			170
#define KEY_CONFIG		171	/* AL Consumer Control Configuration */
#define KEY_HOMEPAGE		172	/* AC Home */
#define KEY_REFRESH		173	/* AC Refresh */
#define KEY_EXIT		174	/* AC Exit */
#define KEY_MOVE		175
#define KEY_EDIT		176
#define KEY_SCROLLUP		177
#define KEY_SCROLLDOWN		178
#define KEY_KPLEFTPAREN		179
#define KEY_KPRIGHTPAREN	180
#define KEY_NEW			181	/* AC New */
#define KEY_REDO		182	/* AC Redo/Repeat */

#define KEY_F13			183
#define KEY_F14			184
#define KEY_F15			185
#define KEY_F16			186
#define KEY_F17			187
#define KEY_F18			188
#define KEY_F19			189
#define KEY_F20			190
#define KEY_F21			191
#define KEY_F22			192
#define KEY_F23			193
#define KEY_F24			194

#define KEY_PLAYCD		200
#define KEY_PAUSECD		201
#define KEY_PROG3		202
#define KEY_PROG4		203
#define KEY_DASHBOARD		204	/* AL Dashboard */
#define KEY_SUSPEND		205
#define KEY_CLOSE		206	/* AC Close */
#define KEY_PLAY		207
#define KEY_FASTFORWARD		208
#define KEY_BASSBOOST		209
#define KEY_PRINT		210	/* AC Print */
#define KEY_HP			211
#define KEY_CAMERA		212
#define KEY_SOUND		213
#define KEY_QUESTION		214
#define KEY_EMAIL		215
#define KEY_CHAT		216
#define KEY_SEARCH		217
#define KEY_CONNECT		218
#define KEY_FINANCE		219	/* AL Checkbook/Finance */
#define KEY_SPORT		220
#define KEY_SHOP		221
#define KEY_ALTERASE		222
#define KEY_CANCEL		223	/* AC Cancel */
#define KEY_BRIGHTNESSDOWN	224
#define KEY_BRIGHTNESSUP	225
#define KEY_MEDIA		226

#define KEY_SWITCHVIDEOMODE	227	/* Cycle between available video
					   outputs (Monitor/LCD/TV-out/etc) */
#define KEY_KBDILLUMTOGGLE	228
#define KEY_KBDILLUMDOWN	229
#define KEY_KBDILLUMUP		230

#define KEY_SEND		231	/* AC Send */
#define KEY_REPLY		232	/* AC Reply */
#define KEY_FORWARDMAIL		233	/* AC Forward Msg */
#define KEY_SAVE		234	/* AC Save */
#define KEY_DOCUMENTS		235

#define KEY_BATTERY		236

#define KEY_BLUETOOTH		237
#define KEY_WLAN		238
#define KEY_UWB			239

#define KEY_UNKNOWN		240

#define KEY_VIDEO_NEXT		241	/* drive next video source */
#define KEY_VIDEO_PREV		242	/* drive previous video source */
#define KEY_BRIGHTNESS_CYCLE	243	/* brightness up, after max is min */
#define KEY_BRIGHTNESS_ZERO	244	/* brightness off, use ambient */
#define KEY_DISPLAY_OFF		245	/* display device to off state */

#define KEY_WIMAX		246

/*
*  UART Settings 
*/
#define	TTY_DEV	"/dev/ttyAMA0"	//端口路径
#define TTY_SPD	(115200)
#define TIMEOUT_SEC(buflen,baud) (buflen*20/baud+2)  //接收超时
#define TIMEOUT_USEC 0

/*
*  UART cmd definition 
*/
#define CMD_LEN_MAX     5

#define CMD_HEADER_REQ        0x4A
#define CMD_HEADER_REQ_NR     0x4B
#define CMD_HEADER_RSP        0x6D
#define CMD_HEADER_RPT        0x55

#define CMD_CODE_MASK         0xE0
#define CMD_CODE_MASK_IR      0x01
#define CMD_CODE_MASK_AU      0x02
#define CMD_CODE_MASK_HDMI    0x03
#define CMD_CODE_MASK_DLPC    0x04
#define CMD_CODE_MASK_ACC     0x05
#define CMD_CODE_MASK_THERM   0x06
#define CMD_CODE_MASK_MOTOR   0x07
#define CMD_CODE_MASK_FAN     0x08

#define SET_CODE(m,c)         ((uint8_t)(m<<5|c))
#define GET_MASK(m)           ((m&CMD_CODE_MASK)>>5)
#define GET_OP(m)             (m&~CMD_CODE_MASK)

#define REMOTE_MI_POWER  0xFF01
#define REMOTE_MI_UP     0xFF02
#define REMOTE_MI_DOWN   0xFF03
#define REMOTE_MI_LEFT   0xFF04
#define REMOTE_MI_RIGHT  0xFF05
#define REMOTE_MI_OK     0xFF06
#define REMOTE_MI_HOME   0xFF07
#define REMOTE_MI_BACK   0xFF08
#define REMOTE_MI_MENU   0xFF09
#define REMOTE_MI_PLUS   0xFF0A
#define REMOTE_MI_MINUS  0xFF0B

typedef enum
{
  CMD_OP_IR_CODE                            = 0x00
}ir_op_t;


/* Globals */
static int uinp_fd = -1;
struct uinput_user_dev uinp; // uInput device structure
struct input_event event; // Input device structure
int RS232_fd;

void proc_uart_read(void);
void proc_uart_write(void);
void proc_uart_cmd(unsigned char *cmd);

/* Calc the CRC32 */
static const unsigned int crc32tab[] = {
 0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL,
 0x076dc419L, 0x706af48fL, 0xe963a535L, 0x9e6495a3L,
 0x0edb8832L, 0x79dcb8a4L, 0xe0d5e91eL, 0x97d2d988L,
 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L, 0x90bf1d91L,
 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
 0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L,
 0x136c9856L, 0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL,
 0x14015c4fL, 0x63066cd9L, 0xfa0f3d63L, 0x8d080df5L,
 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L, 0xa2677172L,
 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
 0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L,
 0x32d86ce3L, 0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L,
 0x26d930acL, 0x51de003aL, 0xc8d75180L, 0xbfd06116L,
 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L, 0xb8bda50fL,
 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
 0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL,
 0x76dc4190L, 0x01db7106L, 0x98d220bcL, 0xefd5102aL,
 0x71b18589L, 0x06b6b51fL, 0x9fbfe4a5L, 0xe8b8d433L,
 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL, 0xe10e9818L,
 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
 0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL,
 0x6c0695edL, 0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L,
 0x65b0d9c6L, 0x12b7e950L, 0x8bbeb8eaL, 0xfcb9887cL,
 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L, 0xfbd44c65L,
 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
 0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL,
 0x4369e96aL, 0x346ed9fcL, 0xad678846L, 0xda60b8d0L,
 0x44042d73L, 0x33031de5L, 0xaa0a4c5fL, 0xdd0d7cc9L,
 0x5005713cL, 0x270241aaL, 0xbe0b1010L, 0xc90c2086L,
 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
 0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L,
 0x59b33d17L, 0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL,
 0xedb88320L, 0x9abfb3b6L, 0x03b6e20cL, 0x74b1d29aL,
 0xead54739L, 0x9dd277afL, 0x04db2615L, 0x73dc1683L,
 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
 0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L,
 0xf00f9344L, 0x8708a3d2L, 0x1e01f268L, 0x6906c2feL,
 0xf762575dL, 0x806567cbL, 0x196c3671L, 0x6e6b06e7L,
 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL, 0x67dd4accL,
 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
 0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L,
 0xd1bb67f1L, 0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL,
 0xd80d2bdaL, 0xaf0a1b4cL, 0x36034af6L, 0x41047a60L,
 0xdf60efc3L, 0xa867df55L, 0x316e8eefL, 0x4669be79L,
 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
 0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL,
 0xc5ba3bbeL, 0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L,
 0xc2d7ffa7L, 0xb5d0cf31L, 0x2cd99e8bL, 0x5bdeae1dL,
 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL, 0x026d930aL,
 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
 0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L,
 0x92d28e9bL, 0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L,
 0x86d3d2d4L, 0xf1d4e242L, 0x68ddb3f8L, 0x1fda836eL,
 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L, 0x18b74777L,
 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
 0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L,
 0xa00ae278L, 0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L,
 0xa7672661L, 0xd06016f7L, 0x4969474dL, 0x3e6e77dbL,
 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L, 0x37d83bf0L,
 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
 0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L,
 0xbad03605L, 0xcdd70693L, 0x54de5729L, 0x23d967bfL,
 0xb3667a2eL, 0xc4614ab8L, 0x5d681b02L, 0x2a6f2b94L,
 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL, 0x2d02ef8dL
};
 
unsigned int calc_crc32( unsigned char *buf, unsigned int size)
{
     unsigned int i, crc;
     crc = 0xFFFFFFFF;
 
     for (i = 0; i < size; i++)
      crc = crc32tab[(crc ^ buf[i]) & 0xff] ^ (crc >> 8);
 
     return crc^0xFFFFFFFF;
}

/* Calc the CRC8 */
unsigned char calc_crc8(unsigned char *s, unsigned int len)
{
	const unsigned char *data = s;
	unsigned crc = 0;
	int i, j;
	for (j = len; j; j--, data++) 
	{
		crc ^= (*data << 8);
		for (i = 8; i; i--) 
		{
			if (crc & 0x8000)
				crc ^= (0x1070 << 3);
			crc <<= 1;
		}
	}

	return (unsigned char)(crc >> 8);
}

/* Setup the uinput device */
int setup_uinput_device()
{
	// Temporary variable
	int i=0;
	// Open the input device
	uinp_fd = open("/dev/uinput", O_WRONLY | O_NDELAY);
	if (uinp_fd == NULL)
	{
		printf("Unable to open UINPUT device.\n");
		return -1;
	}
	memset(&uinp,0,sizeof(uinp)); // Intialize the uInput device to NULL
	strncpy(uinp.name, "PolyVision Touch Screen", UINPUT_MAX_NAME_SIZE);
	uinp.id.version = 4;
	uinp.id.bustype = BUS_USB;
	
	// Setup the uinput device
	ioctl(uinp_fd, UI_SET_EVBIT, EV_KEY);
	ioctl(uinp_fd, UI_SET_EVBIT, EV_REL);
	ioctl(uinp_fd, UI_SET_RELBIT, REL_X);
	ioctl(uinp_fd, UI_SET_RELBIT, REL_Y);

	for (i=0; i < 256; i++) 
	{
		ioctl(uinp_fd, UI_SET_KEYBIT, i);
	}

	ioctl(uinp_fd, UI_SET_KEYBIT, BTN_MOUSE);
	ioctl(uinp_fd, UI_SET_KEYBIT, BTN_TOUCH);
	ioctl(uinp_fd, UI_SET_KEYBIT, BTN_MOUSE);
	ioctl(uinp_fd, UI_SET_KEYBIT, BTN_LEFT);
	ioctl(uinp_fd, UI_SET_KEYBIT, BTN_MIDDLE);
	ioctl(uinp_fd, UI_SET_KEYBIT, BTN_RIGHT);
	ioctl(uinp_fd, UI_SET_KEYBIT, BTN_FORWARD);
	ioctl(uinp_fd, UI_SET_KEYBIT, BTN_BACK);
	
	/* Create input device into input sub-system */
	write(uinp_fd, &uinp, sizeof(uinp));
	if (ioctl(uinp_fd, UI_DEV_CREATE))
	{
		printf("Unable to create UINPUT device.\n");
		return -1;
	}
	
	return 1;
}

void send_click_events()
{
	// Move pointer to (0,0) location
	memset(&event, 0, sizeof(event));
	gettimeofday(&event.time, NULL);
	event.type = EV_REL;
	event.code = REL_X;

	event.value = 100;
	write(uinp_fd, &event, sizeof(event));
	event.type = EV_REL;
	event.code = REL_Y;
	event.value = 100;
	write(uinp_fd, &event, sizeof(event));
	event.type = EV_SYN;
	event.code = SYN_REPORT;
	event.value = 0;
	write(uinp_fd, &event, sizeof(event));
	// Report BUTTON CLICK - PRESS event
	memset(&event, 0, sizeof(event));
	gettimeofday(&event.time, NULL);
	event.type = EV_KEY;
	event.code = BTN_LEFT;
	event.value = 1;
	write(uinp_fd, &event, sizeof(event));
	event.type = EV_SYN;
	event.code = SYN_REPORT;
	event.value = 0;
	write(uinp_fd, &event, sizeof(event));
	// Report BUTTON CLICK - RELEASE event
	memset(&event, 0, sizeof(event));
	gettimeofday(&event.time, NULL);
	event.type = EV_KEY;
	event.code = BTN_LEFT;
	event.value = 0;
	write(uinp_fd, &event, sizeof(event));
	event.type = EV_SYN;
	event.code = SYN_REPORT;
	event.value = 0;
	write(uinp_fd, &event, sizeof(event));
}

void send_a_button(char VK)
{
	// Report BUTTON CLICK - PRESS event
	char aa,bb,cc,dd;

	memset(&event, 0, sizeof(event));

	gettimeofday(&event.time, NULL);

	event.type = EV_KEY;
	event.code = VK;
	event.value = 1;
	aa=write(uinp_fd, &event, sizeof(event));

	event.type = EV_SYN;
	event.code = SYN_REPORT;
	event.value = 0;
	bb=write(uinp_fd, &event, sizeof(event));

	// Report BUTTON CLICK - RELEASE event
	memset(&event, 0, sizeof(event));

	gettimeofday(&event.time, NULL);

	event.type = EV_KEY;
	event.code = VK;
	event.value = 0;
	cc=write(uinp_fd, &event, sizeof(event));

	event.type = EV_SYN;
	event.code = SYN_REPORT;
	event.value = 0;
	dd=write(uinp_fd, &event, sizeof(event));

	printf("BUTTON CLICK EVENT IS SUCCEED:%d,%d,%d,%d\n",aa,bb,cc,dd);
}

/*******************************************
 *	波特率转换函数（请确认是否正确）
********************************************/
int convbaud(unsigned long int baudrate)
{
	switch(baudrate)
	{
	case 2400:
		return B2400;
	case 4800:
		return B4800;
	case 9600:
		return B9600;
	case 19200:
		return B19200;
	case 38400:
		return B38400;
	case 57600:
		return B57600;
	case 115200:
		return B115200;
	default:
		return B9600;
	}
}

/*******************************************
 *	Setup comm attr
 *	fdcom: 串口文件描述符，pportinfo: 待设置的端口信息（请确认）
 *
********************************************/
int PortSet(int fdcom, const pportinfo_t pportinfo)
{
	struct termios termios_old, termios_new;
	int 	baudrate, tmp;
	char	databit, stopbit, parity, fctl;

	bzero(&termios_old, sizeof(termios_old));
	bzero(&termios_new, sizeof(termios_new));
	cfmakeraw(&termios_new);
	tcgetattr(fdcom, &termios_old);			//get the serial port attributions
	/*------------设置端口属性----------------*/
	//baudrates
	baudrate = convbaud(pportinfo -> baudrate);
	cfsetispeed(&termios_new, baudrate);		//填入串口输入端的波特率
	cfsetospeed(&termios_new, baudrate);		//填入串口输出端的波特率
	termios_new.c_cflag |= CLOCAL;			//控制模式，保证程序不会成为端口的占有者
	termios_new.c_cflag |= CREAD;			//控制模式，使能端口读取输入的数据

	// 控制模式，flow control
	fctl = pportinfo-> fctl;
	switch(fctl){
		case '0':{
			termios_new.c_cflag &= ~CRTSCTS;		//no flow control
		}break;
		case '1':{
			termios_new.c_cflag |= CRTSCTS;			//hardware flow control
		}break;
		case '2':{
			termios_new.c_iflag |= IXON | IXOFF |IXANY;	//software flow control
		}break;
	}

	//控制模式，data bits
	termios_new.c_cflag &= ~CSIZE;		//控制模式，屏蔽字符大小位
	databit = pportinfo -> databit;
	switch(databit){
		case '5':
			termios_new.c_cflag |= CS5;
		case '6':
			termios_new.c_cflag |= CS6;
		case '7':
			termios_new.c_cflag |= CS7;
		default:
			termios_new.c_cflag |= CS8;
	}

	//控制模式 parity check
	parity = pportinfo -> parity;
	switch(parity){
		case '0':{
			termios_new.c_cflag &= ~PARENB;		//no parity check
		}break;
		case '1':{
			termios_new.c_cflag |= PARENB;		//odd check
			termios_new.c_cflag &= ~PARODD;
		}break;
		case '2':{
			termios_new.c_cflag |= PARENB;		//even check
			termios_new.c_cflag |= PARODD;
		}break;
	}

	//控制模式，stop bits
	stopbit = pportinfo -> stopbit;
	if(stopbit == '2'){
		termios_new.c_cflag |= CSTOPB;	//2 stop bits
	}
	else{
		termios_new.c_cflag &= ~CSTOPB;	//1 stop bits
	}

	//other attributions default
	termios_new.c_oflag &= ~OPOST;			//输出模式，原始数据输出
	termios_new.c_cc[VMIN]  = CMD_LEN_MAX;			//控制字符, 所要读取字符的最小数量
	termios_new.c_cc[VTIME] = 0;			//控制字符, 读取第一个字符的等待时间	unit: (1/10)second

	tcflush(fdcom, TCIFLUSH);				//溢出的数据可以接收，但不读
	tmp = tcsetattr(fdcom, TCSANOW, &termios_new);	//设置新属性，TCSANOW：所有改变立即生效	tcgetattr(fdcom, &termios_old);
	
	return(tmp);
}

/*******************************************
 *	Open serial port
 *	tty: 端口号 ttyS0, ttyS1, ....
 *	返回值为串口文件描述符
********************************************/
int PortOpen(pportinfo_t pportinfo)
{
	int fdcom;	//串口文件描述符

	//fdcom = open(ptty, O_RDWR | O_NOCTTY | O_NONBLOCK | O_NDELAY);
	fdcom = open(TTY_DEV, O_RDWR | O_NOCTTY);

	return (fdcom);
}

/*******************************************
 *	Close serial port
********************************************/
void PortClose(int fdcom)
{
	close(fdcom);
}

/********************************************
 *	send data
 *	fdcom: 串口描述符，data: 待发送数据，datalen: 数据长度
 *	返回实际发送长度
*********************************************/
int PortSend(int fdcom, char *data, int datalen)
{
	int len = 0;

	len = write(fdcom, data, datalen);	//实际写入的长度
	if(len == datalen){
		return (len);
	}
	else{
		tcflush(fdcom, TCOFLUSH);
		return -1;
	}
}

/*******************************************
 *	receive data
 *	返回实际读入的字节数
 *
********************************************/
int PortRecv(int fdcom, char *data, int datalen, int baudrate)
{
	int readlen, fs_sel;
	fd_set	fs_read;
	struct timeval tv_timeout;

	FD_ZERO(&fs_read);
	FD_SET(fdcom, &fs_read);
	tv_timeout.tv_sec = TIMEOUT_SEC(datalen, baudrate);
	tv_timeout.tv_usec = TIMEOUT_USEC;

	fs_sel = select(fdcom+1, &fs_read, NULL, NULL, &tv_timeout);
	if(fs_sel){
		readlen = read(fdcom, data, datalen);
		return(readlen);
	}
	else{
		return(-1);
	}

	return (readlen);
}

/*******************************************
 *	proc_uart_read
 *
********************************************/
void proc_uart_read(void)
{
	int readn=0;
	unsigned char rBuf[64];
	
	readn = read(RS232_fd, rBuf, CMD_LEN_MAX);
	//readn=PortRecv(RS232_fd, rBuf, 1, portinfo.baudrate);
	printf("UART data %x %x %x %x %x\n", rBuf[0],rBuf[1],rBuf[2],rBuf[3],rBuf[4]);
	if (CMD_LEN_MAX == readn)
	{
		unsigned int crc;
		
		//crc = calc_crc32(rBuf, 4);
		//printf("UART data %x %x %x %x %x\n", rBuf[0],rBuf[1],rBuf[2],rBuf[3],rBuf[4]);
		if (1)//(crc == rBuf[4])
		{
			proc_uart_cmd(rBuf);
		}
		else
		{
			printf("UART data CRC error 0x%08x,0x%02x\n", crc,rBuf[4]);
		}
	}
}

/*******************************************
 *	proc_uart_write
 *
********************************************/
void proc_uart_write(void)
{
#if 0
	int writen=0;
	rBuf[0]=0xAA;
	readn = write(RS232_fd, rBuf, 1);
	printf("UART write %d, %d\n", RS232_fd, readn);
#endif
}

/*******************************************
 *	proc_uart_cmd
 *
********************************************/
void proc_uart_cmd(unsigned char *cmd)
{
	if (CMD_CODE_MASK_IR == GET_MASK(cmd[1]))
	{
		if (CMD_OP_IR_CODE == GET_OP(cmd[1]))
		{
			unsigned short key = (cmd[2]|(cmd[3]<<8));
			switch (key)
			{
			//case  REMOTE_MI_POWER  : send_a_button(cmd[3]);
			case  REMOTE_MI_UP     : send_a_button(KEY_UP);break;
			case  REMOTE_MI_DOWN   : send_a_button(KEY_DOWN);break;
			case  REMOTE_MI_LEFT   : send_a_button(KEY_LEFT);break;
			case  REMOTE_MI_RIGHT  : send_a_button(KEY_RIGHT);break;
			case  REMOTE_MI_OK     : send_a_button(KEY_ENTER);break;
			//case  REMOTE_MI_HOME   : send_a_button(cmd[3]);
			case  REMOTE_MI_BACK   : send_a_button(KEY_ESC);break;
			//case  REMOTE_MI_MENU   : send_a_button(cmd[3]);break;
			case  REMOTE_MI_PLUS   : send_a_button(KEY_F9);break;
			case  REMOTE_MI_MINUS  : send_a_button(KEY_F10);break;
			}
		}
	}
	
	if (CMD_HEADER_REQ_NR == cmd[0])
	{
	}
}

/* This function will open the uInput device. Please make
sure that you have inserted the uinput.ko into kernel. */
int main()
{
	// Return an error if device not found.

	if (setup_uinput_device() < 0)
	{
		printf("Unable to setup UINPUT device\n");
		exit(1);
	}

	portinfo_t portinfo ={
							'0',                          	// print prompt after receiving
							TTY_SPD,                      	// baudrate: 115200
							'8',                          	// databit: 8
							'0',                          	// debug: off
							'0',                          	// echo: off
							'0',                          	// flow control: software
							'2',                          	// default tty: COM1
							'0',                          	// parity: none
							'1',                          	// stopbit: 1
							 0    	                  	// reserved
						};

	if((RS232_fd=PortOpen(&portinfo))<0)
	{
		printf("Error: PortOpen error.\n");
		exit(1);
	}

	if(0!=PortSet(RS232_fd, &portinfo))
	{
		printf("Error: PortSet error.\n");
		exit(1);
	}

	while(1)
	{
		proc_uart_read();
		//proc_uart_write();
		//printf("vkey running...\n");
	};

	/* Close the UINPUT device */
	ioctl(uinp_fd, UI_DEV_DESTROY);
	close(uinp_fd);
	close(RS232_fd );
}

