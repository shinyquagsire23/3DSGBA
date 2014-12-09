#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <3ds.h>
#include <sys/dirent.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include <fcntl.h>
#include <vector>
#include <algorithm>

#include "../system.h"
#include "../port.h"
#include "../gba.h"
#include "../memory.h"
#include "../sound.h"
#include "../globals.h"

static unsigned char asciiData[128][8] = {
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
        { 0x00, 0x3E, 0x41, 0x55, 0x41, 0x55, 0x49, 0x3E },
        { 0x00, 0x3E, 0x7F, 0x6B, 0x7F, 0x6B, 0x77, 0x3E },
        { 0x00, 0x22, 0x77, 0x7F, 0x7F, 0x3E, 0x1C, 0x08 },
        { 0x00, 0x08, 0x1C, 0x3E, 0x7F, 0x3E, 0x1C, 0x08 },
        { 0x00, 0x08, 0x1C, 0x2A, 0x7F, 0x2A, 0x08, 0x1C },
        { 0x00, 0x08, 0x1C, 0x3E, 0x7F, 0x3E, 0x08, 0x1C },
        { 0x00, 0x00, 0x1C, 0x3E, 0x3E, 0x3E, 0x1C, 0x00 },
        { 0xFF, 0xFF, 0xE3, 0xC1, 0xC1, 0xC1, 0xE3, 0xFF },
        { 0x00, 0x00, 0x1C, 0x22, 0x22, 0x22, 0x1C, 0x00 },
        { 0xFF, 0xFF, 0xE3, 0xDD, 0xDD, 0xDD, 0xE3, 0xFF },
        { 0x00, 0x0F, 0x03, 0x05, 0x39, 0x48, 0x48, 0x30 },
        { 0x00, 0x08, 0x3E, 0x08, 0x1C, 0x22, 0x22, 0x1C },
        { 0x00, 0x18, 0x14, 0x10, 0x10, 0x30, 0x70, 0x60 },
        { 0x00, 0x0F, 0x19, 0x11, 0x13, 0x37, 0x76, 0x60 },
        { 0x00, 0x08, 0x2A, 0x1C, 0x77, 0x1C, 0x2A, 0x08 },
        { 0x00, 0x60, 0x78, 0x7E, 0x7F, 0x7E, 0x78, 0x60 },
        { 0x00, 0x03, 0x0F, 0x3F, 0x7F, 0x3F, 0x0F, 0x03 },
        { 0x00, 0x08, 0x1C, 0x2A, 0x08, 0x2A, 0x1C, 0x08 },
        { 0x00, 0x66, 0x66, 0x66, 0x66, 0x00, 0x66, 0x66 },
        { 0x00, 0x3F, 0x65, 0x65, 0x3D, 0x05, 0x05, 0x05 },
        { 0x00, 0x0C, 0x32, 0x48, 0x24, 0x12, 0x4C, 0x30 },
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F, 0x7F, 0x7F },
        { 0x00, 0x08, 0x1C, 0x2A, 0x08, 0x2A, 0x1C, 0x3E },
        { 0x00, 0x08, 0x1C, 0x3E, 0x7F, 0x1C, 0x1C, 0x1C },
        { 0x00, 0x1C, 0x1C, 0x1C, 0x7F, 0x3E, 0x1C, 0x08 },
        { 0x00, 0x08, 0x0C, 0x7E, 0x7F, 0x7E, 0x0C, 0x08 },
        { 0x00, 0x08, 0x18, 0x3F, 0x7F, 0x3F, 0x18, 0x08 },
        { 0x00, 0x00, 0x00, 0x70, 0x70, 0x70, 0x7F, 0x7F },
        { 0x00, 0x00, 0x14, 0x22, 0x7F, 0x22, 0x14, 0x00 },
        { 0x00, 0x08, 0x1C, 0x1C, 0x3E, 0x3E, 0x7F, 0x7F },
        { 0x00, 0x7F, 0x7F, 0x3E, 0x3E, 0x1C, 0x1C, 0x08 },
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
        { 0x00, 0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18 },
        { 0x00, 0x36, 0x36, 0x14, 0x00, 0x00, 0x00, 0x00 },
        { 0x00, 0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36 },
        { 0x00, 0x08, 0x1E, 0x20, 0x1C, 0x02, 0x3C, 0x08 },
        { 0x00, 0x60, 0x66, 0x0C, 0x18, 0x30, 0x66, 0x06 },
        { 0x00, 0x3C, 0x66, 0x3C, 0x28, 0x65, 0x66, 0x3F },
        { 0x00, 0x18, 0x18, 0x18, 0x30, 0x00, 0x00, 0x00 },
        { 0x00, 0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06 },
        { 0x00, 0x60, 0x30, 0x18, 0x18, 0x18, 0x30, 0x60 },
        { 0x00, 0x00, 0x36, 0x1C, 0x7F, 0x1C, 0x36, 0x00 },
        { 0x00, 0x00, 0x08, 0x08, 0x3E, 0x08, 0x08, 0x00 },
        { 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x30, 0x60 },
        { 0x00, 0x00, 0x00, 0x00, 0x3C, 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x60 },
        { 0x00, 0x00, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x00 },
        { 0x00, 0x3C, 0x66, 0x6E, 0x76, 0x66, 0x66, 0x3C },
        { 0x00, 0x18, 0x18, 0x38, 0x18, 0x18, 0x18, 0x7E },
        { 0x00, 0x3C, 0x66, 0x06, 0x0C, 0x30, 0x60, 0x7E },
        { 0x00, 0x3C, 0x66, 0x06, 0x1C, 0x06, 0x66, 0x3C },
        { 0x00, 0x0C, 0x1C, 0x2C, 0x4C, 0x7E, 0x0C, 0x0C },
        { 0x00, 0x7E, 0x60, 0x7C, 0x06, 0x06, 0x66, 0x3C },
        { 0x00, 0x3C, 0x66, 0x60, 0x7C, 0x66, 0x66, 0x3C },
        { 0x00, 0x7E, 0x66, 0x0C, 0x0C, 0x18, 0x18, 0x18 },
        { 0x00, 0x3C, 0x66, 0x66, 0x3C, 0x66, 0x66, 0x3C },
        { 0x00, 0x3C, 0x66, 0x66, 0x3E, 0x06, 0x66, 0x3C },
        { 0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x00 },
        { 0x00, 0x00, 0x18, 0x18, 0x00, 0x18, 0x18, 0x30 },
        { 0x00, 0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06 },
        { 0x00, 0x00, 0x00, 0x3C, 0x00, 0x3C, 0x00, 0x00 },
        { 0x00, 0x60, 0x30, 0x18, 0x0C, 0x18, 0x30, 0x60 },
        { 0x00, 0x3C, 0x66, 0x06, 0x1C, 0x18, 0x00, 0x18 },
        { 0x00, 0x38, 0x44, 0x5C, 0x58, 0x42, 0x3C, 0x00 },
        { 0x00, 0x3C, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66 },
        { 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x66, 0x66, 0x7C },
        { 0x00, 0x3C, 0x66, 0x60, 0x60, 0x60, 0x66, 0x3C },
        { 0x00, 0x7C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x7C },
        { 0x00, 0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x7E },
        { 0x00, 0x7E, 0x60, 0x60, 0x7C, 0x60, 0x60, 0x60 },
        { 0x00, 0x3C, 0x66, 0x60, 0x60, 0x6E, 0x66, 0x3C },
        { 0x00, 0x66, 0x66, 0x66, 0x7E, 0x66, 0x66, 0x66 },
        { 0x00, 0x3C, 0x18, 0x18, 0x18, 0x18, 0x18, 0x3C },
        { 0x00, 0x1E, 0x0C, 0x0C, 0x0C, 0x6C, 0x6C, 0x38 },
        { 0x00, 0x66, 0x6C, 0x78, 0x70, 0x78, 0x6C, 0x66 },
        { 0x00, 0x60, 0x60, 0x60, 0x60, 0x60, 0x60, 0x7E },
        { 0x00, 0x63, 0x77, 0x7F, 0x6B, 0x63, 0x63, 0x63 },
        { 0x00, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x63, 0x63 },
        { 0x00, 0x3C, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C },
        { 0x00, 0x7C, 0x66, 0x66, 0x66, 0x7C, 0x60, 0x60 },
        { 0x00, 0x3C, 0x66, 0x66, 0x66, 0x6E, 0x3C, 0x06 },
        { 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x78, 0x6C, 0x66 },
        { 0x00, 0x3C, 0x66, 0x60, 0x3C, 0x06, 0x66, 0x3C },
        { 0x00, 0x7E, 0x5A, 0x18, 0x18, 0x18, 0x18, 0x18 },
        { 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3E },
        { 0x00, 0x66, 0x66, 0x66, 0x66, 0x66, 0x3C, 0x18 },
        { 0x00, 0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63 },
        { 0x00, 0x63, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x63 },
        { 0x00, 0x66, 0x66, 0x66, 0x3C, 0x18, 0x18, 0x18 },
        { 0x00, 0x7E, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x7E },
        { 0x00, 0x1E, 0x18, 0x18, 0x18, 0x18, 0x18, 0x1E },
        { 0x00, 0x00, 0x60, 0x30, 0x18, 0x0C, 0x06, 0x00 },
        { 0x00, 0x78, 0x18, 0x18, 0x18, 0x18, 0x18, 0x78 },
        { 0x00, 0x08, 0x14, 0x22, 0x41, 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F },
        { 0x00, 0x0C, 0x0C, 0x06, 0x00, 0x00, 0x00, 0x00 },
        { 0x00, 0x00, 0x00, 0x3C, 0x06, 0x3E, 0x66, 0x3E },
        { 0x00, 0x60, 0x60, 0x60, 0x7C, 0x66, 0x66, 0x7C },
        { 0x00, 0x00, 0x00, 0x3C, 0x66, 0x60, 0x66, 0x3C },
        { 0x00, 0x06, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3E },
        { 0x00, 0x00, 0x00, 0x3C, 0x66, 0x7E, 0x60, 0x3C },
        { 0x00, 0x1C, 0x36, 0x30, 0x30, 0x7C, 0x30, 0x30 },
        { 0x00, 0x00, 0x3E, 0x66, 0x66, 0x3E, 0x06, 0x3C },
        { 0x00, 0x60, 0x60, 0x60, 0x7C, 0x66, 0x66, 0x66 },
        { 0x00, 0x00, 0x18, 0x00, 0x18, 0x18, 0x18, 0x3C },
        { 0x00, 0x0C, 0x00, 0x0C, 0x0C, 0x6C, 0x6C, 0x38 },
        { 0x00, 0x60, 0x60, 0x66, 0x6C, 0x78, 0x6C, 0x66 },
        { 0x00, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18 },
        { 0x00, 0x00, 0x00, 0x63, 0x77, 0x7F, 0x6B, 0x6B },
        { 0x00, 0x00, 0x00, 0x7C, 0x7E, 0x66, 0x66, 0x66 },
        { 0x00, 0x00, 0x00, 0x3C, 0x66, 0x66, 0x66, 0x3C },
        { 0x00, 0x00, 0x7C, 0x66, 0x66, 0x7C, 0x60, 0x60 },
        { 0x00, 0x00, 0x3C, 0x6C, 0x6C, 0x3C, 0x0D, 0x0F },
        { 0x00, 0x00, 0x00, 0x7C, 0x66, 0x66, 0x60, 0x60 },
        { 0x00, 0x00, 0x00, 0x3E, 0x40, 0x3C, 0x02, 0x7C },
        { 0x00, 0x00, 0x18, 0x18, 0x7E, 0x18, 0x18, 0x18 },
        { 0x00, 0x00, 0x00, 0x66, 0x66, 0x66, 0x66, 0x3E },
        { 0x00, 0x00, 0x00, 0x00, 0x66, 0x66, 0x3C, 0x18 },
        { 0x00, 0x00, 0x00, 0x63, 0x6B, 0x6B, 0x6B, 0x3E },
        { 0x00, 0x00, 0x00, 0x66, 0x3C, 0x18, 0x3C, 0x66 },
        { 0x00, 0x00, 0x00, 0x66, 0x66, 0x3E, 0x06, 0x3C },
        { 0x00, 0x00, 0x00, 0x3C, 0x0C, 0x18, 0x30, 0x3C },
        { 0x00, 0x0E, 0x18, 0x18, 0x30, 0x18, 0x18, 0x0E },
        { 0x00, 0x18, 0x18, 0x18, 0x00, 0x18, 0x18, 0x18 },
        { 0x00, 0x70, 0x18, 0x18, 0x0C, 0x18, 0x18, 0x70 },
        { 0x00, 0x00, 0x00, 0x3A, 0x6C, 0x00, 0x00, 0x00 },
        { 0x00, 0x08, 0x1C, 0x36, 0x63, 0x41, 0x41, 0x7F }
};

typedef struct  {
   const char* romtitle;
   const char* romid;
   int flashSize;
   int saveType;
   int rtcEnabled;
   int mirroringEnabled;
   int useBios;
   u32 idle_loop;
} ini_t;

static const ini_t gbaover[256] = {
        //romtitle,							    	romid	flash	save	rtc	mirror	bios		idle loop
        {"2 Games in 1 - Dragon Ball Z - The Legacy of Goku I & II (USA)",	"BLFE",	0,	1,	0,	0,	0, 0},
        {"2 Games in 1 - Dragon Ball Z - Buu's Fury + Dragon Ball GT - Transformation (USA)", "BUFE", 0, 1, 0, 0, 0, 0},
        {"Boktai - The Sun Is in Your Hand (Europe)(En,Fr,De,Es,It)",		"U3IP",	0,	0,	1,	0,	0, 0},
        {"Boktai - The Sun Is in Your Hand (USA)",				"U3IE",	0,	0,	1,	0,	0, 0},
        {"Boktai 2 - Solar Boy Django (USA)",					"U32E",	0,	0,	1,	0,	0, 0},
        {"Boktai 2 - Solar Boy Django (Europe)(En,Fr,De,Es,It)",		"U32P",	0,	0,	1,	0,	0, 0},
        {"Bokura no Taiyou - Taiyou Action RPG (Japan)",			"U3IJ",	0,	0,	1,	0,	0, 0},
        {"Card e-Reader+ (Japan)",						"PSAJ",	131072,	0,	0,	0,	0, 0},
        {"Classic NES Series - Bomberman (USA, Europe)",			"FBME",	0,	1,	0,	1,	0, 0},
        {"Classic NES Series - Castlevania (USA, Europe)",			"FADE",	0,	1,	0,	1,	0, 0},
        {"Classic NES Series - Donkey Kong (USA, Europe)",			"FDKE",	0,	1,	0,	1,	0, 0},
        {"Classic NES Series - Dr. Mario (USA, Europe)",			"FDME",	0,	1,	0,	1,	0, 0},
        {"Classic NES Series - Excitebike (USA, Europe)",			"FEBE",	0,	1,	0,	1,	0, 0},
        {"Classic NES Series - Legend of Zelda (USA, Europe)",			"FZLE",	0,	1,	0,	1,	0, 0},
        {"Classic NES Series - Ice Climber (USA, Europe)",			"FICE",	0,	1,	0,	1,	0, 0},
        {"Classic NES Series - Metroid (USA, Europe)",				"FMRE",	0,	1,	0,	1,	0, 0},
        {"Classic NES Series - Pac-Man (USA, Europe)",				"FP7E",	0,	1,	0,	1,	0, 0},
        {"Classic NES Series - Super Mario Bros. (USA, Europe)",		"FSME",	0,	1,	0,	1,	0, 0},
        {"Classic NES Series - Xevious (USA, Europe)",				"FXVE",	0,	1,	0,	1,	0, 0},
        {"Classic NES Series - Zelda II - The Adventure of Link (USA, Europe)",	"FLBE",	0,	1,	0,	1,	0, 0},
        {"Digi Communication 2 - Datou! Black Gemagema Dan (Japan)",		"BDKJ",	0,	1,	0,	0,	0, 0},
        {"e-Reader (USA)",							"PSAE",	131072,	0,	0,	0,	0, 0},
        {"Dragon Ball GT - Transformation (USA)",				"BT4E",	0,	1,	0,	0,	0, 0},
        {"Dragon Ball Z - Buu's Fury (USA)",					"BG3E",	0,	1,	0,	0,	0, 0},
        {"Dragon Ball Z - Taiketsu (Europe)(En,Fr,De,Es,It)",			"BDBP",	0,	1,	0,	0,	0, 0},
        {"Dragon Ball Z - Taiketsu (USA)",					"BDBE",	0,	1,	0,	0,	0, 0},
        {"Dragon Ball Z - The Legacy of Goku II International (Japan)",		"ALFJ",	0,	1,	0,	0,	0, 0},
        {"Dragon Ball Z - The Legacy of Goku II (Europe)(En,Fr,De,Es,It)",	"ALFP", 0,	1,	0,	0,	0, 0},
        {"Dragon Ball Z - The Legacy of Goku II (USA)",				"ALFE",	0,	1,	0,	0,	0, 0},
        {"Dragon Ball Z - The Legacy Of Goku (Europe)(En,Fr,De,Es,It)",		"ALGP",	0,	1,	0,	0,	0, 0},
        {"Dragon Ball Z - The Legacy of Goku (USA)",				"ALGE",	131072,	1,	0,	0,	0, 0},
        {"F-Zero - Climax (Japan)",						"BFTJ",	131072,	0,	0,	0,	0, 0},
        {"Famicom Mini Vol. 01 - Super Mario Bros. (Japan)",			"FMBJ",	0,	1,	0,	1,	0, 0},
        {"Famicom Mini Vol. 12 - Clu Clu Land (Japan)",				"FCLJ",	0,	1,	0,	1,	0, 0},
        {"Famicom Mini Vol. 13 - Balloon Fight (Japan)",			"FBFJ",	0,	1,	0,	1,	0, 0},
        {"Famicom Mini Vol. 14 - Wrecking Crew (Japan)",			"FWCJ",	0,	1,	0,	1,	0, 0},
        {"Famicom Mini Vol. 15 - Dr. Mario (Japan)",				"FDMJ",	0,	1,	0,	1,	0, 0},
        {"Famicom Mini Vol. 16 - Dig Dug (Japan)",				"FTBJ",	0,	1,	0,	1,	0, 0},
        {"Famicom Mini Vol. 17 - Takahashi Meijin no Boukenjima (Japan)",	"FTBJ",	0,	1,	0,	1,	0, 0},
        {"Famicom Mini Vol. 18 - Makaimura (Japan)",				"FMKJ",	0,	1,	0,	1,	0, 0},
        {"Famicom Mini Vol. 19 - Twin Bee (Japan)",				"FTWJ",	0,	1,	0,	1,	0, 0},
        {"Famicom Mini Vol. 20 - Ganbare Goemon! Karakuri Douchuu (Japan)",	"FGGJ",	0,	1,	0,	1,	0, 0},
        {"Famicom Mini Vol. 21 - Super Mario Bros. 2 (Japan)",			"FM2J",	0,	1,	0,	1,	0, 0},
        {"Famicom Mini Vol. 22 - Nazo no Murasame Jou (Japan)",			"FNMJ",	0,	1,	0,	1,	0, 0},
        {"Famicom Mini Vol. 23 - Metroid (Japan)",				"FMRJ",	0,	1,	0,	1,	0, 0},
        {"Famicom Mini Vol. 24 - Hikari Shinwa - Palthena no Kagami (Japan)",	"FPTJ",	0,	1,	0,	1,	0, 0},
        {"Famicom Mini Vol. 25 - The Legend of Zelda 2 - Link no Bouken (Japan)","FLBJ",0,	1,	0,	1,	0, 0},
        {"Famicom Mini Vol. 26 - Famicom Mukashi Banashi - Shin Onigashima - Zen Kou Hen (Japan)","FFMJ",0,1,0,	1,	0, 0},
        {"Famicom Mini Vol. 27 - Famicom Tantei Club - Kieta Koukeisha - Zen Kou Hen (Japan)","FTKJ",0,1,0,	1,	0, 0},
        {"Famicom Mini Vol. 28 - Famicom Tantei Club Part II - Ushiro ni Tatsu Shoujo - Zen Kou Hen (Japan)","FTUJ",0,1,0,1,0, 0},
        {"Famicom Mini Vol. 29 - Akumajou Dracula (Japan)",			"FADJ",	0,	1,	0,	1,	0, 0},
        {"Famicom Mini Vol. 30 - SD Gundam World - Gachapon Senshi Scramble Wars (Japan)","FSDJ",0,1,	0,	1,	0, 0},
        {"Game Boy Wars Advance 1+2 (Japan)",					"BGWJ",	131072,	0,	0,	0,	0, 0},
        {"Golden Sun - The Lost Age (USA)",					"AGFE",	65536,	0,	0,	1,	0, 0},
        {"Golden Sun (USA)",							"AGSE",	65536,	0,	0,	1,	0, 0},
        {"Koro Koro Puzzle - Happy Panechu! (Japan)",				"KHPJ",	0,	4,	0,	0,	0, 0},
        {"Mario vs. Donkey Kong (Europe)",					"BM5P",	0,	3,	0,	0,	0, 0},
        {"Pocket Monsters - Emerald (Japan)",					"BPEJ",	131072,	0,	1,	0,	0, 0x080008ce},
        {"Pocket Monsters - Fire Red (Japan)",					"BPRJ",	131072,	0,	0,	0,	0, 0x080008b2},
        {"Pocket Monsters - Leaf Green (Japan)",				"BPGJ",	131072,	0,	0,	0,	0, 0},
        {"Pocket Monsters - Ruby (Japan)",					"AXVJ",	131072,	0,	1,	0,	0, 0},
        {"Pocket Monsters - Sapphire (Japan)",					"AXPJ",	131072,	0,	1,	0,	0, 0},
        {"Pokemon Mystery Dungeon - Red Rescue Team (USA, Australia)",		"B24E",	131072,	0,	0,	0,	0, 0},
        {"Pokemon Mystery Dungeon - Red Rescue Team (En,Fr,De,Es,It)",		"B24P",	131072,	0,	0,	0,	0, 0},
        {"Pokemon - Blattgruene Edition (Germany)",				"BPGD",	131072,	0,	0,	0,	0, 0},
        {"Pokemon - Edicion Rubi (Spain)",					"AXVS",	131072,	0,	1,	0,	0, 0},
        {"Pokemon - Edicion Esmeralda (Spain)",					"BPES",	131072,	0,	1,	0,	0, 0},
        {"Pokemon - Edicion Rojo Fuego (Spain)",				"BPRS",	131072,	1,	0,	0,	0, 0},
        {"Pokemon - Edicion Verde Hoja (Spain)",				"BPGS",	131072,	1,	0,	0,	0, 0},
        {"Pokemon - Eidicion Zafiro (Spain)",					"AXPS",	131072,	0,	1,	0,	0, 0},
        {"Pokemon - Emerald Version (USA, Europe)",				"BPEE",	131072,	0,	1,	0,	0, 0x080008ce},
        {"Pokemon - Feuerrote Edition (Germany)",				"BPRD",	131072,	0,	0,	0,	0, 0},
        {"Pokemon - Fire Red Version (USA, Europe)",				"BPRE",	131072,	0,	0,	0,	0, 0x080008b2}, //NOTE: Idle loop is only for v1.0
        {"Pokemon - Leaf Green Version (USA, Europe)",				"BPGE",	131072,	0,	0,	0,	0, 0},
        {"Pokemon - Rubin Edition (Germany)",					"AXVD",	131072,	0,	1,	0,	0, 0},
        {"Pokemon - Ruby Version (USA, Europe)",				"AXVE",	131072,	0,	1,	0,	0, 0},
        {"Pokemon - Sapphire Version (USA, Europe)",				"AXPE",	131072,	0,	1,	0,	0, 0},
        {"Pokemon - Saphir Edition (Germany)",					"AXPD",	131072,	0,	1,	0,	0, 0},
        {"Pokemon - Smaragd Edition (Germany)",					"BPED",	131072,	0,	1,	0,	0, 0},
        {"Pokemon - Version Emeraude (France)",					"BPEF",	131072,	0,	1,	0,	0, 0},
        {"Pokemon - Version Rouge Feu (France)",				"BPRF",	131072,	0,	0,	0,	0, 0},
        {"Pokemon - Version Rubis (France)",					"AXVF",	131072,	0,	1,	0,	0, 0},
        {"Pokemon - Version Saphir (France)",					"AXPF",	131072,	0,	1,	0,	0, 0},
        {"Pokemon - Version Vert Feuille (France)",				"BPGF",	131072,	0,	0,	0,	0, 0},
        {"Pokemon - Versione Rubino (Italy)",					"AXVI",	131072,	0,	1,	0,	0, 0},
        {"Pokemon - Versione Rosso Fuoco (Italy)",				"BPRI",	131072,	0,	0,	0,	0, 0},
        {"Pokemon - Versione Smeraldo (Italy)",					"BPEI",	131072,	0,	1,	0,	0, 0},
        {"Pokemon - Versione Verde Foglia (Italy)",				"BPGI",	131072,	0,	0,	0,	0, 0},
        {"Pokemon - Versione Zaffiro (Italy)",					"AXPI",	131072,	0,	1,	0,	0, 0},
        {"Rockman EXE 4.5 - Real Operation (Japan)",				"BR4J",	0,	0,	1,	0,	0, 0},
        {"Rocky (Europe)(En,Fr,De,Es,It)",					"AROP",	0,	1,	0,	0,	0, 0},
        {"Rocky (USA)(En,Fr,De,Es,It)",						"AR8e",	0,	1,	0,	0,	0, 0},
        {"Sennen Kazoku (Japan)",						"BKAJ",	131072,	0,	1,	0,	0, 0},
        {"Shin Bokura no Taiyou - Gyakushuu no Sabata (Japan)",			"U33J",	0,	1,	1,	0,	0, 0},
        {"Super Mario Advance 4 (Japan)",					"AX4J",	131072,	0,	0,	0,	0, 0},
        {"Super Mario Advance 4 - Super Mario Bros. 3 (Europe)(En,Fr,De,Es,It)","AX4P",	131072,	0,	0,	0,	0, 0},
        {"Super Mario Advance 4 - Super Mario Bros 3 - Super Mario Advance 4 v1.1 (USA)","AX4E",131072,0,0,0,0, 0},
        {"Top Gun - Combat Zones (USA)(En,Fr,De,Es,It)",			"A2YE",	0,	5,	0,	0,	0, 0},
        {"Yoshi's Universal Gravitation (Europe)(En,Fr,De,Es,It)",		"KYGP",	0,	4,	0,	0,	0, 0},
        {"Yoshi no Banyuuinryoku (Japan)",					"KYGJ",	0,	4,	0,	0,	0, 0},
        {"Yoshi - Topsy-Turvy (USA)",						"KYGE",	0,	1,	0,	0,	0, 0},
        {"Yu-Gi-Oh! GX - Duel Academy (USA)",					"BYGE",	0,	2,	0,	0,	1, 0},
        {"Yu-Gi-Oh! - Ultimate Masters - 2006 (Europe)(En,Jp,Fr,De,Es,It)",	"BY6P",	0,	2,	0,	0,	0, 0},
        {"Zoku Bokura no Taiyou - Taiyou Shounen Django (Japan)",		"U32J",	0,	0,	1,	0,	0}
};

PAD_KEY buttonMap[10] = {
        KEY_A,
        KEY_B,
        KEY_SELECT,
        KEY_START,
        KEY_RIGHT,
        KEY_LEFT,
        KEY_UP,
        KEY_DOWN,
        KEY_R,
        KEY_L
};

bool emuPause = false;
bool emuStarted = false;

static unsigned serialize_size = 0;
static unsigned has_frame;
static unsigned g_audio_frames;
static unsigned g_video_frames;
static unsigned char hasAudio = 0;
extern uint64_t joy;

char* gamePath;

u8* fb;
u16 fbWidth, fbHeight = 0;
int xOffset, yOffset;
u32 bgr_555_to_bgr_888_table[32 * 32 * 32];

uint8_t save_buf[0x20000 + 0x2000];	/* Workaround for broken-by-design GBA save semantics. */

static unsigned save_size = sizeof(save_buf);

void initLookup() {
    for(u8 r5 = 0; r5 < 32; r5++) {
        for(u8 g5 = 0; g5 < 32; g5++) {
            for(u8 b5 = 0; b5 < 32; b5++) {
                u8 r8 = (u8) ((r5 * 527 + 23) >> 6);
                u8 g8 = (u8) ((g5 * 527 + 23) >> 6);
                u8 b8 = (u8) ((b5 * 527 + 23) >> 6);
                u32 bgr555 = (b5 << 10) | (g5 << 5) | r5;
                u32 bgr888 = (b8 << 16) | (g8 << 8) | r8;
                bgr_555_to_bgr_888_table[bgr555] = bgr888;
            }
        }
    }
}

int screen_get_index(int x, int y, u16 fbWidth, u16 fbHeight) {
    return ((fbWidth - y - 1) + x * fbWidth) * 3;
}

void screen_fill(u8* fb, u16 fbWidth, u16 fbHeight, int x, int y, int width, int height, u8 r, u8 g, u8 b) {
    if(fb == NULL) {
        return;
    }

    int swidth = fbHeight;
    int sheight = fbWidth;
    if(x + width < 0 || y + height < 0 || x >= swidth || y >= sheight) {
        return;
    }

    if(x < 0) {
        width += x;
        x = 0;
    }

    if(y < 0) {
        height += y;
        y = 0;
    }

    if(x + width >= swidth){
        width = swidth - x;
    }

    if(y + height >= sheight){
        height = sheight - y;
    }

    u8 colorLine[height * 3];
    for(int ly = 0; ly < height; ly++) {
        colorLine[ly * 3 + 0] = b;
        colorLine[ly * 3 + 1] = g;
        colorLine[ly * 3 + 2] = r;
    }

    u8* fbAddr = fb + screen_get_index(x, y, fbWidth, fbHeight) - (height * 3);
    for(int dx = 0; dx < width; dx++) {
        memcpy(fbAddr, colorLine, (size_t) (height * 3));
        fbAddr += sheight * 3;
    }
}

void screen_draw_char(u8* fb, u16 fbWidth, u16 fbHeight, char c, int x, int y, u8 r, u8 g, u8 b) {
    if(fb == NULL) {
        return;
    }

    unsigned char* data = asciiData[(int) c];
    for(int cy = 0; cy < 8; cy++) {
        if(y + cy < 0 || y + cy >= fbWidth) {
            continue;
        }

        unsigned char l = data[cy];
        for(int cx = 0; cx < 8; cx++) {
            if(x + cx < 0 || x + cx >= fbHeight) {
                continue;
            }

            if((0b10000000 >> cx) & l) {
                u8* ptr = &fb[screen_get_index(x + cx, y + cy, fbWidth, fbHeight)];
                *(ptr + 0) = b;
                *(ptr + 1) = g;
                *(ptr + 2) = r;
            }
        }
    }
}

void screen_draw_string(u8* fb, u16 fbWidth, u16 fbHeight, const char* string, int x, int y, u8 r, u8 g, u8 b) {
    if(fb == NULL) {
        return;
    }

    int len = (int) strlen(string);
    int cx = x;
    int cy = y;
    for(int i = 0; i < len; i++) {
        char c = string[i];
        if(c == '\n') {
            cx = x;
            cy += 8;
        }

        if(cx > fbHeight - 8) {
            break;
        }

        screen_draw_char(fb, fbWidth, fbHeight, c, cx, cy, r, g, b);
        cx += 8;
    }
}

size_t emulator_serialize_size(void)
{
   return serialize_size;
}

bool emulator_serialize(void *data, size_t size)
{
   return (bool) CPUWriteState((uint8_t*)data, size);
}

bool emulator_unserialize(const void *data, size_t size)
{
   return CPUReadState((uint8_t*)data, size);
}

static bool scan_area(const uint8_t *data, unsigned size)
{
   for (unsigned i = 0; i < size; i++)
      if (data[i] != 0xff)
         return true;

   return false;
}

static void adjust_save_ram()
{
   if (scan_area(save_buf, 512) &&
         !scan_area(save_buf + 512, sizeof(save_buf) - 512))
   {
      save_size = 512;
      systemMessage("Detecting EEprom 8kbit\n");
   }
   else if (scan_area(save_buf, 0x2000) &&
         !scan_area(save_buf + 0x2000, sizeof(save_buf) - 0x2000))
   {
      save_size = 0x2000;
      systemMessage("Detecting EEprom 64kbit\n");
   }

   else if (scan_area(save_buf, 0x10000) &&
         !scan_area(save_buf + 0x10000, sizeof(save_buf) - 0x10000))
   {
      save_size = 0x10000;
      systemMessage("Detecting Flash 512kbit\n");
   }
   else if (scan_area(save_buf, 0x20000) &&
         !scan_area(save_buf + 0x20000, sizeof(save_buf) - 0x20000))
   {
      save_size = 0x20000;
      systemMessage("Detecting Flash 1Mbit\n");
   }
   else
      systemMessage("Did not detect any particular SRAM type.\n");

   if (save_size == 512 || save_size == 0x2000)
      eepromData = save_buf;
   else if (save_size == 0x10000 || save_size == 0x20000)
      flashSaveMemory = save_buf;
}

static void load_image_preferences(void)
{
   char buffer[5];
   buffer[0] = rom[0xac];
   buffer[1] = rom[0xad];
   buffer[2] = rom[0xae];
   buffer[3] = rom[0xaf];
   buffer[4] = 0;
   systemMessage("GameID in ROM is: %s\n", buffer);

   bool found = false;
   int found_no = 0;

   for(int i = 0; i < 256; i++)
   {
       if(gbaover[i].romid != NULL) {
           if(!strcmp(gbaover[i].romid, buffer)) {
               found = true;
               found_no = i;
               break;
           }
       }
   }

   if(found)
   {
      systemMessage("Found ROM in vba-over list.\n");
      enableRtc = (bool) gbaover[found_no].rtcEnabled;
      if(gbaover[found_no].flashSize != 0) {
          flashSize = gbaover[found_no].flashSize;
      } else {
          flashSize = 65536;
      }

      cpuSaveType = gbaover[found_no].saveType;
      mirroringEnable = (bool) gbaover[found_no].mirroringEnabled;

      // Patch out idle loops with lsl r0, r0, #0x0 (nop)
      if(gbaover[found_no].idle_loop) {
         u32 offset = gbaover[found_no].idle_loop;
         systemMessage("Patching idle loop at offset %d.", offset);
         rom[offset] = 0;
         rom[(offset + 1) & 0x1FFFFFF] = 0;
      }
   }

   systemMessage("RTC = %d.\n", enableRtc);
   systemMessage("flashSize = %d.\n", flashSize);
   systemMessage("cpuSaveType = %d.\n", cpuSaveType);
   systemMessage("mirroringEnable = %d.\n", mirroringEnable);
}

static void gba_init(void)
{
   cpuSaveType = 0;
   flashSize = 0x10000;
   enableRtc = false;
   mirroringEnable = false;

   load_image_preferences();

   if(flashSize == 0x10000 || flashSize == 0x20000)
      flashSetSize(flashSize);

   if(enableRtc)
      rtcEnable(enableRtc);

   doMirroring(mirroringEnable);

   soundSetSampleRate(32000);

   CPUInit(0, false);
   CPUReset();

   soundReset();

   uint8_t * state_buf = (uint8_t*) systemAlloc(2000000);
   serialize_size = CPUWriteState(state_buf, 2000000);
   systemFree(state_buf);
}

void emulator_init(void)
{
    initLookup();
   memset(save_buf, 0xff, sizeof(save_buf));
   adjust_save_ram();
}

void emulator_cleanup(void)
{
	CPUCleanUp();
}

void emulator_reset(void)
{
   CPUReset();
}

void emulator_run(void) {
   hidScanInput();
   if(hidKeysDown() & KEY_TOUCH) {
       emuPause = true;
       return;
   }

    if(hidKeysDown() & KEY_X) {
        char* fname = strdup(gamePath);
        fname[strlen(fname) - 3] = 'r';
        fname[strlen(fname) - 2] = 'a';
        fname[strlen(fname) - 1] = 'm';

        size_t size = emulator_serialize_size();
        void* data = systemAlloc(size);
        emulator_serialize(data, size);
        int fd = open(fname, O_WRONLY | O_CREAT);
        write(fd, data, size);
        close(fd);
        systemFree(data);
    }

    if(hidKeysDown() & KEY_Y) {
        char* fname = strdup(gamePath);
        fname[strlen(fname) - 3] = 'r';
        fname[strlen(fname) - 2] = 'a';
        fname[strlen(fname) - 1] = 'm';

        FILE* fd = fopen(fname, "r");
        if(!fd) {
            return;
        }

        fseek(fd, 0, SEEK_END);
        size_t size = (size_t) ftell(fd);
        fseek(fd, 0, SEEK_SET);
        void* data = systemAlloc(size);
        fread(data, size, 1, fd);
        fclose(fd);
        emulator_unserialize(data, size);
        systemFree(data);
    }

   joy = 0;
   for(unsigned i = 0; i < 10; i++) {
      joy |= ((bool) (hidKeysHeld() & buttonMap[i])) << i;
   }

   has_frame = 0;
   while(!has_frame) {
      CPULoop();
   }
}

bool emulator_load_game(const char* path) {
   bool ret = (bool) CPULoadRom(path);
    gamePath = strdup(path);
   gba_init();

    char* fname = strdup(gamePath);
    fname[strlen(fname) - 3] = 's';
    fname[strlen(fname) - 2] = 'a';
    fname[strlen(fname) - 1] = 'v';

    FILE* fd = fopen(fname, "r");
    if(fd) {
        fclose(fd);
        CPUReadBatteryFile(fname);
    }

   return ret;
}

void emulator_unload_game(void) {
   systemMessage("[VBA] Sync stats: Audio frames: %u, Video frames: %u, AF/VF: %.2f\n", g_audio_frames, g_video_frames, (float)g_audio_frames / g_video_frames);
   gamePath = NULL;
    g_audio_frames = 0;
   g_video_frames = 0;
}

void systemOnWriteDataToSoundBuffer(s16* finalWave, int length)
{
    if(hasAudio)
        CSND_playsound(0x8, CSND_LOOP_DISABLE, CSND_ENCODING_PCM16, 22050, (u32*)finalWave, NULL, 1600, 2, 0);
    g_audio_frames += length >> 1;
}

u32* systemGetPixels() {
    return (u32*) gfxGetFramebuffer(GFX_TOP, GFX_LEFT, &fbWidth, &fbHeight);
}

u16 systemGetScreenWidth() {
    if(fbHeight == 0) {
        systemGetPixels();
    }

    return fbHeight;
}

u16 systemGetScreenHeight() {
    if(fbWidth == 0) {
        systemGetPixels();
    }

    return fbWidth;
}

u16 systemGetScreenDepth() {
    return 3;
}

void systemPrepareDraw() {
    fb = (u8*) systemGetPixels();
    xOffset = (fbHeight - 240) / 2;
    yOffset = (fbWidth - 160) / 2;
}

void systemDrawPixel(int x, int y, u32 color) {
    u8* ptr = &fb[((fbWidth - (y + yOffset) - 1) + (x + xOffset) * fbWidth) * 3];
    u32 bgr = bgr_555_to_bgr_888_table[color & 0xFFFF];
    *(ptr + 0) = (u8) ((bgr >> 16) & 0xFF);
    *(ptr + 1) = (u8) ((bgr >> 8) & 0xFF);
    *(ptr + 2) = (u8) (bgr & 0xFF);
}

void systemFlushScreen()
{
    gfxFlushBuffers();
    gfxSwapBuffers();
    gspWaitForVBlank();
   g_video_frames++;
   has_frame = 1;
}

void systemMessage(const char* fmt, ...)
{
   char buffer[256];
   va_list ap;
   va_start(ap, fmt);
   vsnprintf(buffer, sizeof(buffer), fmt, ap);
   svcOutputDebugString(buffer, strlen(buffer));
   va_end(ap);
}

void* systemAlloc(u32 size) {
    return linearAlloc(size);
}

void systemFree(void* ptr) {
    linearFree(ptr);
}

char* getPath(char* dir, char* file) {
    char* path = (char*) malloc(strlen(dir) + strlen(file) + 2);
    snprintf(path, strlen(dir) + strlen(file) + 2, "%s/%s", dir, file);
    return path;
}

bool isDirectory(char* path) {
    DIR *dir = opendir(path);
    if(!dir) {
        return false;
    }

    closedir(dir);
    return true;
}

bool extMatches(const char* path, const char* extension) {
    const char *dot = strrchr(path, '.');
    if(!dot || dot == path) {
        return false;
    }

    return strcmp(dot + 1, extension) == 0;
}

struct alphabetize {
    inline bool operator() (char* a, char* b) {
        return strcasecmp(a, b) < 0;
    }
};

std::vector<char*>* getContents(char* directory) {
    std::vector<char*>* contents = new std::vector<char*>();
    char buf[strlen(directory) + 2];
    snprintf(buf, sizeof(buf), "%s/", directory);
    DIR *dir = opendir(buf);
    if(dir != NULL) {
        for(int i = 0; i < 256; i++) {
            struct dirent *ent = readdir(dir);
            if(ent == NULL) {
                break;
            }

            char* path = getPath(directory, ent->d_name);
            if(isDirectory(path) || extMatches(path, "gba")) {
                contents->push_back(strdup(ent->d_name));
            }
        }

        closedir(dir);
        std::sort(contents->begin(), contents->end(), alphabetize());
    } else {
        systemMessage("Could not read path %s", directory);
        return NULL;
    }

    return contents;
}

const char* chooseGame() {
    char* currDir = "sdmc:";
    std::vector<char*>* contents = getContents(currDir);
    char* game = NULL;
    int cursor = 0;
    int scroll = 0;
    int horizScroll = 0;
    while(aptMainLoop()) {
        hidScanInput();
        if(hidKeysDown() & KEY_START && hidKeysDown() & KEY_SELECT) {
            return "////EXIT////";
        }

        if(emuStarted && hidKeysDown() & KEY_TOUCH) {
            break;
        }

        if(hidKeysDown() & KEY_A) {
            char* curr = contents->at(cursor);
            if(strcmp(curr, ".") != 0) {
                if(strcmp(curr, "..") == 0) {
                    char* slash = strrchr(currDir, '/');
                    if(slash) {
                        currDir[strlen(currDir) - strlen(slash)] = '\0';
                        delete(contents);
                        contents = getContents(currDir);
                        cursor = 0;
                    }
                } else {
                    char* path = getPath(currDir, curr);
                    if(isDirectory(path)) {
                        currDir = path;
                        delete(contents);
                        contents = getContents(currDir);
                        cursor = 0;
                    } else {
                        game = path;
                        break;
                    }
                }
            }
        }

        if(hidKeysDown() & KEY_B) {
            char* slash = strrchr(currDir, '/');
            if(slash) {
                currDir[strlen(currDir) - strlen(slash)] = '\0';
                delete(contents);
                contents = getContents(currDir);
                cursor = 0;
            }
        }

        if(hidKeysDown() & KEY_DOWN && cursor < contents->size() - 1) {
            cursor++;
            if(cursor - scroll >= 20) {
                scroll++;
            }

            horizScroll = 0;
        }

        if(hidKeysDown() & KEY_UP && cursor > 0) {
            cursor--;
            if(cursor - scroll < 0) {
                scroll--;
            }

            horizScroll = 0;
        }

        u16 fbWidth, fbHeight;
        u8* fb = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, &fbWidth, &fbHeight);
        screen_fill(fb, fbWidth, fbHeight, 0, 0, fbHeight, fbWidth, 0, 0, 0);
        int i = 0;
        for(std::vector<char*>::iterator it = contents->begin() + scroll; it != contents->end(); it++) {
            u8 color = 255;
            int offset = 0;
            if(i + scroll == cursor) {
                screen_fill(fb, fbWidth, fbHeight, 0, i * 12, fbHeight, 8, 255, 255, 255);
                color = 0;
                u32 width = strlen(*it) * 8;
                if(width > fbHeight) {
                    horizScroll -= 2;
                    if(-horizScroll + fbHeight >= width) {
                        horizScroll = 0;
                    }
                }

                offset = horizScroll;
            }

            screen_draw_string(fb, fbWidth, fbHeight, *it, offset, i * 12, color, color, color);
            i++;
            if(i >= 20) {
                break;
            }
        }

        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    delete(contents);

    // Get those buffers nice and clear.
    for(int i = 0; i < 2; i++) {
        u16 fbWidth, fbHeight;
        u8* fb = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, &fbWidth, &fbHeight);
        screen_fill(fb, fbWidth, fbHeight, 0, 0, fbHeight, fbWidth, 0, 0, 0);
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }

    return game;
}

int main(int argv, char** argc) {
    srvInit();
    aptInit();
    hidInit(NULL);
    gfxInit();
    fsInit();
    sdmcInit();
    hasAudio = !CSND_initialize(NULL);
    emulator_init();

    while(aptMainLoop()) {
        if(emuStarted) {
            emulator_run();
        }

        if(!emuStarted || emuPause) {
            emuPause = false;
            const char* game = chooseGame();
            if(game != NULL) {
                if(strcmp(game, "////EXIT////") == 0) {
                    break;
                }

                if(emuStarted) {
                    emulator_unload_game();
                    emulator_cleanup();
                }

                if(!emulator_load_game(game)) {
                    systemMessage("Failed to load %s", game);
                    return 0;
                }

                emuStarted = true;
            } else if(!emuStarted) {
                break;
            }
        }
    }

    CSND_shutdown();
    sdmcExit();
    fsExit();
    gfxExit();
    hidExit();
    aptExit();
    srvExit();
    return 0;
}
