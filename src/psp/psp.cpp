 #define PASSWORD_XORED "watashihaos9xpspdesu."

/*******************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002 Gary Henderson (gary.henderson@ntlworld.com) and
                            Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2001 - 2004 John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2004 Brad Jorsch (anomie@users.sourceforge.net),
                            funkyass (funkyass@spam.shaw.ca),
                            Joel Yliluoma (http://iki.fi/bisqwit/)
                            Kris Bleakley (codeviolation@hotmail.com),
                            Matthew Kendora,
                            Nach (n-a-c-h@users.sourceforge.net),
                            Peter Bortas (peter@bortas.org) and
                            zones (kasumitokoduck@yahoo.com)

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003 zsKnight (zsknight@zsnes.com),
                            _Demo_ (_demo_@zsnes.com), and Nach

  C4 C++ code
  (c) Copyright 2003 Brad Jorsch

  DSP-1 emulator code
  (c) Copyright 1998 - 2004 Ivar (ivar@snes9x.com), _Demo_, Gary Henderson,
                            John Weidman, neviksti (neviksti@hotmail.com),
                            Kris Bleakley, Andreas Naive

  DSP-2 emulator code
  (c) Copyright 2003 Kris Bleakley, John Weidman, neviksti, Matthew Kendora, and
                     Lord Nightmare (lord_nightmare@users.sourceforge.net

  OBC1 emulator code
  (c) Copyright 2001 - 2004 zsKnight, pagefault (pagefault@zsnes.com) and
                            Kris Bleakley
  Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code
  (c) Copyright 2002 Matthew Kendora with research by
                     zsKnight, John Weidman, and Dark Force

  S-DD1 C emulator code
  (c) Copyright 2003 Brad Jorsch with research by
                     Andreas Naive and John Weidman

  S-RTC C emulator code
  (c) Copyright 2001 John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003 Feather, Kris Bleakley, John Weidman and Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003 zsKnight, _Demo_, and pagefault

  Super FX C emulator code
  (c) Copyright 1997 - 1999 Ivar, Gary Henderson and John Weidman


  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004 Marcus Comstedt (marcus@mc.pp.se)


  Specific ports contains the works of other authors. See headers in
  individual files.

  Snes9x homepage: http://www.snes9x.com

  Permission to use, copy, modify and distribute Snes9x in both binary and
  source form, for non-commercial purposes, is hereby granted without fee,
  providing that this license information and copyright notice appear with
  all copies and any derived work.

  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software.

  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes
  charging money for Snes9x or software derived from Snes9x.

  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.

  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
*******************************************************************************/
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "snes9x.h"
#include "memmap.h"
#include "debug.h"
#include "cpuexec.h"
#include "ppu.h"
#include "snapshot.h"
#include "apu.h"
#include "display.h"
#include "gfx.h"
#include "soundux.h"
#include "spc700.h"
#include "psp/counter.h"
//#include "spc7110.h"


#include "psp/psp.h"
#include "psp/filer.h"
#include "psp/menu.h"
#include "psp/imageio.h"

#include "danzeff.h"


#include "psp/openspc++/os9xZ_openspc.h"

#include "tile_psp.h"

#include <pspaudio.h>

PSP_MODULE_INFO("Snes9X Euphoria", 0, 1, 0);
PSP_MAIN_THREAD_ATTR(THREAD_ATTR_USER | THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(-256);

#ifdef PROFILE
profile_t profile_data;
#endif

#include "psp_net.h"
extern "C" {
#include "cheats.h"



#include "img_jpeg.h"

#include "psp/blitter.h"

#include "message.c"
#include "decrypt.h"

#ifdef HOME_HOOK
#include "homehook.h"
#endif

#ifdef ME_SOUND
#include "me.h"
#endif

uint32 caCRC32(uint8 *array, uint32 size, register uint32 crc32 = 0xFFFFFFFF);
int net_waitpause_state(int show_menu);
void net_send_state();
void net_receive_settings();
void net_send_settings();
void net_flush_net(int to_send);
void before_pause();
void after_pause();
void set_cpu_clock();
#ifndef FW3X
int initUSBdrivers();
int endUSBdrivers();
#endif


float vfpumulsin(float mul,float angle,float range);

int inputBoxOK(char *msg);
void msgBox(const char *msg,int delay_vblank=10);
int msgBoxLines(const char *msg,int delay_vblank=10);
int load_rom_settings(int game_crc32);
int save_rom_settings(int game_crc32,char *name);
void check_battery();

extern int pg_drawframe;

int menu_modified;

int bypass_rom_settings;

char os9x_devicename[256];

//int hires_offset=0;
//int hires_offsetV=0;
//int hires_offset2=0;
//int hires_offsetV2=0;
//int hires_swap=0;


u16 *clut256;
//u16 __attribute__((aligned(16))) clut256[256*3];

int swap_buf;


static struct timeval next1 = { 0, 0 };
static struct timeval next1_autofs = { 0, 0 };

static struct timeval os9x_autosavetimer_tv = {0,0};

char *os9x_shortfilename(char *filename);
char *os9x_filename_ext(char *filename);

extern void image_put(int x0,int y0,IMAGE* img,int fade,int add);

int os9x_load(const char *ext);
int os9x_remove(const char *ext);
int os9x_loadfname(const char *fname);
int os9x_save(const char *ext);
int os9x_S9Xsave(const char *ext);
int os9x_ZSsave(const char *ext);
}


extern struct SCheatData Cheat;


const char *S9xGetSaveFilename( const char *e );

int os9x_apu_ratio,os9x_fpslimit;
int os9x_usballowed;

char str_tmp[256];
char LaunchDir[256];
char romPath[256];
char lastRom[256];
char os9x_viewfile_path[256];

char os9x_nickname[256];
int os9x_timezone,os9x_daylsavings;
int os9x_language=LANGUAGE_ENGLISH; //need to be initialized for early error messages! (before calling getsysparam
int os9x_menumusic,os9x_menufx,os9x_menupadbeep;

IMAGE* bg_img;
int bg_img_mul;
int bg_img_num;

extern "C" {
IMAGE *icons[8];
int icons_col[8];
}

u16 os9x_savestate_mini[128*120];

int os9x_notfirstlaunch;

unsigned int __attribute__((aligned(64))) list[262144*4];

void InitSoundThread();
void StopSoundThread();



#include "gammatab.h"

#define AUTO_FSKIP 10
#define SKEEZIX_FSKIP 11
#define MAX_AUTO_SKIP 9
#define MIN_AUTOFRAME_LAG 200  //0,2 ms

int os9x_netplay,os9x_conId; //conId : 2 is client, 1 is server


uint8  __attribute__((aligned(64))) pkt_recv[NET_PKT_LEN],pkt_send[NET_PKT_LEN];

int os9x_updatepadcpt,os9x_updatepadFrame;
int os9x_padfirstcall;
int os9x_adhoc_active;
int os9x_padindex,os9x_netpadindex;
char os9x_IPaddr[32];
int os9x_vsync;
int os9x_screenLeft,os9x_screenTop,os9x_screenHeight,os9x_screenWidth;
int os9x_renderingpass,os9x_snesheight,os9x_forcepal_ntsc;
int os9x_externstate_mode,os9x_autosavesram;
int os9x_lowbat,os9x_autosavetimer;
int os9x_TurboMode;

int os9x_BG0,os9x_BG1,os9x_BG2,os9x_BG3,os9x_OBJ,os9x_easy,os9x_hack,os9x_applyhacks;

int os9x_inputs[32],os9x_inputs_analog;
int os9x_snespad,os9x_oldsnespad,os9x_specialaction,os9x_specialaction_old;
int os9x_netsnespad[NET_DELAY][5];
int os9x_netcrc32[NET_DELAY][5];
int os9x_netsynclost;
int os9x_oldframe;

int os9x_getnewfile=0;
int os9x_showfps,os9x_showpass;
int os9x_padvalue,os9x_padvalue_ax,os9x_padvalue_ay;

int os9x_cpuclock;
uint32 os9x_ColorsChanged;
uint32 os9x_gammavalue;
int os9x_fastsprite;
int32 os9x_ShowSub;
int os9x_render;
int os9x_softrendering,os9x_smoothing;

int os9x_fskipvalue,os9x_autofskip_SkipFrames;
int os9x_CyclesPercentage;
int os9x_apuenabled;
int* os9x_apuenabled_ptr;
int os9x_DisableHDMA;
int os9x_DisableIRQ;
int os9x_speedlimit;

int os9x_sndfreq;

char rom_filename[256];
char shortrom_filename[64];


int in_emu;

/*******************************/

static struct timeval	s_tvStart;
static int				s_iFrame,s_iFrameAuto,s_iFrameReal;
volatile int				s_TotalFrame;
static int				s_iFlip = 0;


volatile int os9x_paused=0;
volatile int *os9x_paused_ptr;

volatile int			g_bLoop = true;
volatile int				g_sndthread = -1,g_updatethread= -1,g_mainthread=-1;
volatile int g_bSleep;

static int samples_error;
static int current_SoundBuffer=0;
static u32 snd_freqratio,snd_freqerr;
volatile uint16 __attribute__((aligned(64))) SoundBuffer[4][MAX_BUFFER_SIZE];

#ifdef ME_SOUND
typedef struct {
	uint8 *buffer[4];
	int buffer_idx;
	int sample_count;
	int freqratio;
	int freqerr;
	int *os9x_apuenabled_ptr;
	int *os9x_paused_ptr;
	int exit;
	int mixsample_flag;
	//uint8 *apu_ram;
} me_sound_t;

volatile me_sound_t __attribute__((aligned(64))) me_sound_data;
volatile struct me_struct *me_data;
#endif




//static uint8 __attribute__((aligned(64))) 	GFX_Screen[SNES_WIDTH * SNES_HEIGHT_EXTENDED * 2];
static uint8 __attribute__((aligned(64))) 	GFX_SubScreen[SNES_WIDTH * SNES_HEIGHT_EXTENDED * 2];
static uint8 __attribute__((aligned(64))) 	GFX_ZBuffer[SNES_WIDTH * SNES_HEIGHT_EXTENDED * 2];
static uint8 __attribute__((aligned(64))) 	GFX_SubZBuffer[SNES_WIDTH * SNES_HEIGHT_EXTENDED * 2];


#define timercmp(a, b, CMP)	(((a)->tv_sec == (b)->tv_sec) ? ((a)->tv_usec CMP (b)->tv_usec) : ((a)->tv_sec CMP (b)->tv_sec))


void S9xCloseSoundDevice();

extern "C"{

////////////////////////////////////////////////////////////////////////////////////////
// Check for exiting flag
////////////////////////////////////////////////////////////////////////////////////////
int psp_ExitCheck(){
	return !g_bLoop;
}
void ErrorExit(char* msg)
{
	FILE* f;
	char tmp_str[256];
	sprintf(tmp_str,"%serorr.txt",LaunchDir);
	f = fopen(tmp_str,"wb");			
	if (!f){
		//("cannot save settings");
	}
	fwrite(msg,1,strlen(msg),f);
	fclose(f);
    sceKernelExitGame();
}
////////////////////////////////////////////////////////////////////////////////////////
// Read inputs
////////////////////////////////////////////////////////////////////////////////////////
void update_pad(){
	SceCtrlData	ctl;
	int i,j;

	sceCtrlPeekBufferPositive( &ctl, 1 );

	if (!os9x_inputs_analog) {
		if (ctl.Ly >= 0xD0) ctl.Buttons|=PSP_CTRL_DOWN;  // DOWN
		else if (ctl.Ly <= 0x30) ctl.Buttons|=PSP_CTRL_UP;    // UP
		if (ctl.Lx <= 0x30) ctl.Buttons|=PSP_CTRL_LEFT;  // LEFT
		else if (ctl.Lx >= 0xD0) ctl.Buttons|=PSP_CTRL_RIGHT; // RIGHT
	}

	os9x_padvalue=ctl.Buttons;
	os9x_padvalue_ax=ctl.Lx;
	os9x_padvalue_ay=ctl.Ly;

	os9x_oldsnespad=os9x_snespad;
	if (os9x_netplay) {
		for (i=0;i<5;i++) {
			for (j=0;j<NET_DELAY-1;j++) {
				os9x_netsnespad[j][i]=os9x_netsnespad[j+1][i];
				os9x_netcrc32[j][i]=os9x_netcrc32[j+1][i];
			}
		}
	}

	os9x_snespad=0;
	os9x_specialaction=0;

#ifndef HOME_HOOK
	if ((os9x_inputs[PSP_TL_TR])&&(os9x_padvalue & PSP_CTRL_LTRIGGER)&&(os9x_padvalue & PSP_CTRL_RTRIGGER)) {
		os9x_snespad|=os9x_inputs[PSP_TL_TR];
		os9x_padvalue&=~(PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER);
	}
#endif
	if ((os9x_inputs[PSP_TL_START])&&(os9x_padvalue & PSP_CTRL_LTRIGGER)&&(os9x_padvalue & PSP_CTRL_START)) {
		os9x_snespad|=os9x_inputs[PSP_TL_START];
		os9x_padvalue&=~(PSP_CTRL_LTRIGGER|PSP_CTRL_START);
	}
	if ((os9x_inputs[PSP_TR_START])&&(os9x_padvalue & PSP_CTRL_RTRIGGER)&&(os9x_padvalue & PSP_CTRL_START)) {
		os9x_snespad|=os9x_inputs[PSP_TR_START];
		os9x_padvalue&=~(PSP_CTRL_RTRIGGER|PSP_CTRL_START);
	}
	if ((os9x_inputs[PSP_TL_SELECT])&&(os9x_padvalue & PSP_CTRL_LTRIGGER)&&(os9x_padvalue & PSP_CTRL_SELECT)) {
		os9x_snespad|=os9x_inputs[PSP_TL_SELECT];
		os9x_padvalue&=~(PSP_CTRL_LTRIGGER|PSP_CTRL_SELECT);
	}
	if ((os9x_inputs[PSP_TR_SELECT])&&(os9x_padvalue & PSP_CTRL_RTRIGGER)&&(os9x_padvalue & PSP_CTRL_SELECT)) {
		os9x_snespad|=os9x_inputs[PSP_TR_SELECT];
		os9x_padvalue&=~(PSP_CTRL_RTRIGGER|PSP_CTRL_SELECT);
	}
	if ((os9x_inputs[PSP_SELECT_START])&&(os9x_padvalue & PSP_CTRL_SELECT)&&(os9x_padvalue & PSP_CTRL_START)) {
		os9x_snespad|=os9x_inputs[PSP_SELECT_START];
		os9x_padvalue&=~(PSP_CTRL_SELECT|PSP_CTRL_START);
	}

	if (os9x_padvalue & PSP_CTRL_UP) os9x_snespad|=os9x_inputs[PSP_UP];
	if (os9x_padvalue & PSP_CTRL_DOWN) os9x_snespad|=os9x_inputs[PSP_DOWN];
	if (os9x_padvalue & PSP_CTRL_LEFT) os9x_snespad|=os9x_inputs[PSP_LEFT];
	if (os9x_padvalue & PSP_CTRL_RIGHT) os9x_snespad|=os9x_inputs[PSP_RIGHT];

	if (os9x_inputs_analog) {
		if (os9x_padvalue_ay<0x30) os9x_snespad|=os9x_inputs[PSP_AUP];
		else if (os9x_padvalue_ay>0xA0) os9x_snespad|=os9x_inputs[PSP_ADOWN];
		if (os9x_padvalue_ax<0x30) os9x_snespad|=os9x_inputs[PSP_ALEFT];
		else if (os9x_padvalue_ax>0xA0) os9x_snespad|=os9x_inputs[PSP_ARIGHT];
	}

	if (os9x_padvalue & PSP_CTRL_CIRCLE) os9x_snespad|=os9x_inputs[PSP_CIRCLE];
	if (os9x_padvalue & PSP_CTRL_CROSS) os9x_snespad|=os9x_inputs[PSP_CROSS];
	if (os9x_padvalue & PSP_CTRL_SQUARE) os9x_snespad|=os9x_inputs[PSP_SQUARE];
	if (os9x_padvalue & PSP_CTRL_TRIANGLE) os9x_snespad|=os9x_inputs[PSP_TRIANGLE];
	if (os9x_padvalue & PSP_CTRL_START) os9x_snespad|=os9x_inputs[PSP_START];
	if (os9x_padvalue & PSP_CTRL_SELECT) os9x_snespad|=os9x_inputs[PSP_SELECT];
	if (os9x_padvalue & PSP_CTRL_LTRIGGER) os9x_snespad|=os9x_inputs[PSP_TL];
	if (os9x_padvalue & PSP_CTRL_RTRIGGER) os9x_snespad|=os9x_inputs[PSP_TR];
	//if (os9x_padvalue & PSP_CTRL_NOTE) os9x_snespad|=os9x_inputs[PSP_NOTE];


	os9x_specialaction=os9x_snespad&0xFFFF0000;
	os9x_snespad&=0xFFFF;

	// NET STUFF HERE
	//  Standard PACKET is 8 bytes.
	//  Offset   What
	//  0        1 byte : command
	//  cmd = 0
	//  1				 frame number & 255.
	//	2        2*3 bytes : snes pad frame-3,snes pad frame-2,snes pad frame-1
	//
	//  cmd = 1  resync. If sent by client, ask server to do a resync
	//						  		 If sent by server, the savestate follows.
	//
	//  cmd = 2 => going to menu => pause
	//  cmd = 3 => unpause => load state
	//

	//if (os9x_netplay) {
	//}

	os9x_oldframe=os9x_updatepadFrame;
}

////////////////////////////////////////////////////////////////////////////////////////
// Exit callback
////////////////////////////////////////////////////////////////////////////////////////
int ExitCallback(int arg1, int arg2, void *common){
	g_bLoop=0;
#if 0
	// Cleanup the games resources etc (if required)
	//debug_log("Exit Callback");

	//Settings.Paused = TRUE;

	//*os9x_paused_ptr=1;
	//StopSoundThread();
	//scePowerSetClockFrequency(222,222,111);
	sceKernelDelayThread(1000000); //a try to fix hang on 2.5 --Test for R5
	if (g_sndthread>=0) sceKernelTerminateThread(g_sndthread);
	if (g_updatethread>=0) sceKernelTerminateThread(g_updatethread);
	//if (g_mainthread>=0) sceKernelTerminateThread(g_mainthread);

	if (!os9x_lowbat) {
		save_settings();
		if (in_emu==1) {
			Memory.SaveSRAM( (char*)S9xGetSaveFilename(".SRM") );
			//S9xSaveCheatFile( (char*)S9xGetSaveFilename( ".cht" ) );
			save_rom_settings(Memory.ROMCRC32,Memory.ROMName);
		}
		//pgWaitVn(60*1);//give some times to save files
	}

	// S9xCloseSoundDevice();
	//g_bLoop = false;
	// Exit game

	sceKernelExitGame();
#endif
	return 0;
}
}

////////////////////////////////////////////////////////////////////////////////////////
// Power callback
////////////////////////////////////////////////////////////////////////////////////////
int PowerCallback(int unknown, int pwrflags,void *common){
	//debug_log("Power Callback");

	//if (pwrflags & PSP_POWER_CB_HOLD_SWITCH) {
		//can be used for any purpose, debugging, profiling,...
	//}

#ifdef ME_SOUND
	if (pwrflags & PSP_POWER_CB_POWER_SWITCH) {
		if (!g_bSleep) { //going to sleep
			g_bSleep=1;
			/**os9x_paused_ptr=1;
			StopSoundThread();
			sceGuDisplay(0);
			scePowerSetClockFrequency(33,33,16); //set to 12Mhz
			sceKernelDelayThread(1000000*3);*/
		} else { // resuming
			g_bSleep=0;
			//set_cpu_clock();
			/*scePowerSetClockFrequency(222,222,111);
			sceGuDisplay(1);

			Settings.Paused=false;*/
		}
	}
#endif

#ifndef ME_SOUND
	if ((!g_bSleep)&&((pwrflags & PSP_POWER_CB_POWER_SWITCH) || (pwrflags & PSP_POWER_CB_SUSPENDING))) {
		//msgBoxLines("Going into sleep mode\n\nPlease wait...",60);

		g_bSleep=1;
		*os9x_paused_ptr=1;
		StopSoundThread();
		scePowerSetClockFrequency(222,222,111);
		Settings.Paused = TRUE;
	}else if (g_bSleep&&(pwrflags & (PSP_POWER_CB_RESUME_COMPLETE|PSP_POWER_CB_RESUMING))) {
		//msgBoxLines("Resuming from sleep mode\n\nPlease wait...",60);
		g_bSleep=0;
		Settings.Paused=false;
	}
#endif
	int cbid = sceKernelCreateCallback("Power Callback", PowerCallback,NULL);
	scePowerRegisterCallback(0, cbid);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// Thread to create the callbacks and then begin polling
////////////////////////////////////////////////////////////////////////////////////////
int CallbackThread(SceSize args, void *argp){
	int cbid;
	cbid = sceKernelCreateCallback( "Exit Callback",  ExitCallback,NULL );
	sceKernelRegisterExitCallback( cbid );
	cbid = sceKernelCreateCallback( "Power Callback",  PowerCallback,NULL );
	scePowerRegisterCallback( 0, cbid );
	sceKernelSleepThreadCB();
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// Sets up the callback thread and returns its thread id
////////////////////////////////////////////////////////////////////////////////////////
int SetupCallbacks()
{
	int thid = 0;
//#ifndef ME_SOUND
	thid = sceKernelCreateThread( "update_thread", CallbackThread, 0x10, 0xFA0, PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU, 0 );
	if( thid >= 0 ){
		sceKernelStartThread(thid, 0, 0);
	}
//#endif
	return thid;
}

////////////////////////////////////////////////////////////////////////////////////////
// set psp cpu clock
////////////////////////////////////////////////////////////////////////////////////////
void set_cpu_clock(){
#ifndef ME_SOUND
		//Redundant code, we need 333mhz until the emulator is heavilly optimized
		/*switch (os9x_cpuclock){
			case 266:scePowerSetClockFrequency(266,266,133);break;
			case 300:scePowerSetClockFrequency(300,300,150);break;
			case 333:scePowerSetClockFrequency(333,333,166);break;
			default :scePowerSetClockFrequency(222,222,111);
		}*/ 
#endif
}


extern "C" {

////////////////////////////////////////////////////////////////////////////////////////
// debug print
////////////////////////////////////////////////////////////////////////////////////////
void debug_log( const char* message ){
#ifndef RELEASE
	static int	sy = 1;

	pgPrintAllBG( SNES_WIDTH / 8, sy, 0xffff, message );
	sy++;

	if ( sy >= CMAX_Y ){
		int 	x, y;
		uint16*	dest;

		dest = (uint16*)pgGetVramAddr( SNES_WIDTH, 0 );

		for ( y = 0; y < SCREEN_HEIGHT; y++ ){
			for ( x = 0; x < (SCREEN_WIDTH - SNES_WIDTH); x++ ){
				*dest++ = 0;
			}
			dest += (512 - (SCREEN_WIDTH - SNES_WIDTH));
		}
		sy = 1;
	}
#endif // RELEASE
}

void debug_int( const char* message, int value ){
	strcpy( String, message );
	format_int( &String[strlen( String )], value );

	debug_log( String );
}
////////////////////////////////////////////////////////////////////////////////////////
// debug print hex value
////////////////////////////////////////////////////////////////////////////////////////
void debug_hex( int value ){
	int		shift;
	int		val;
	int		i;

	shift = 28;
	for ( i = 0; i < 8; i++ ){
		val = (value >> shift) & 0x0f;
		if ( val < 10 ){
			String[i] = val + '0';
		} else {
			String[i] = val - 10 + 'A';
		}
		shift -= 4;
	}
	String[i] = 0;

	debug_log( String );
}

};

//
// C++ Language
//

////////////////////////////////////////////////////////////////////////////////////////
// SRAM autosaver
////////////////////////////////////////////////////////////////////////////////////////
void S9xAutoSaveSRAM() {
	if (os9x_autosavesram&& (!os9x_lowbat)) Memory.SaveSRAM ((char*)S9xGetSaveFilename(".SRM"));
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
bool8 S9xOpenSoundDevice( int mode, bool8 stereo, int buffer_size )
{
	(stSoundStatus.mute_sound)  = TRUE;
	if ( buffer_size <= 0 ){
		return FALSE;
	}
	(stSoundStatus.sound_switch) = 255;
	(stSoundStatus.buffer_size)  = buffer_size;
	(stSoundStatus.encoded)      = FALSE;
	// Initialize channel and allocate buffer
	(stSoundStatus.sound_fd) = sceAudioChReserve( -1, buffer_size, 0 );
	if ( (stSoundStatus.sound_fd) < 0 ){
		return FALSE;
	}
		(stSoundStatus.buffer_size) *= 2;
		(stSoundStatus.buffer_size) *= 2;
	if ( (stSoundStatus.buffer_size) > MAX_BUFFER_SIZE ){
		(stSoundStatus.buffer_size) = MAX_BUFFER_SIZE;
	}
	samples_error = 0;
	current_SoundBuffer = 0;
	S9xSetPlaybackRate( Settings.SoundPlaybackRate  );
	(stSoundStatus.mute_sound)  = FALSE;
	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void S9xCloseSoundDevice()
{
	if ( (stSoundStatus.sound_fd) >= 0 ){
		sceAudioChRelease( (stSoundStatus.sound_fd) );
		(stSoundStatus.sound_fd) = -1;
	}
}

////////////////////////////////////////////////////////////////////////////////////////
// Synchronous process of me-CPU and main-CPU by ruka

#ifdef ME_SOUND

#define	EVENT_SUSPEND	0
#define EVENT_RESUME	1

#define QUEUE_MASK		0x0F

typedef struct {
	uint8 wSuspend;			// me:w main:r
	uint8 wCallCount;		// me:w mian:r
	uint8  bMainQueuePtr;	// me:r main rw
	uint8  bMeQueuePtr;		// me rw
	uint32 dwTimeout;		// me:rw
	uint8 bMeWorking;		// me:w main:r
	uint8 abQueueData[31];	// me:r main:w
	uint32 dwDummy[6];		// aligned dummy
}PROCESS_EVENT;

volatile PROCESS_EVENT __attribute__((aligned(64))) g_stProcessEvent = {0}; // uncache access only
#endif




////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
#ifdef ME_SOUND

int me_Dummy(int v)
{
	return v;
}

int me_MixSound(me_sound_t *p){
	int i;
	u32 *src,*dst;
	uint8 *apu_ram_save;
//	Uncache_APU_Cycles=Uncache_APU_Cycles+1000;
//apu_init_after_load=0;
//return 0;
	for (;;) {
		if (p->mixsample_flag) {
			int next_buffer_idx = p->buffer_idx + 1;
			//p->buffer_idx++;
			if (next_buffer_idx>2) next_buffer_idx=0;

			src=(u32*)(p->buffer[3]);
			dst=(u32*)(p->buffer[next_buffer_idx]);

			if ((*(p->os9x_apuenabled_ptr)==2)&&(!(*(p->os9x_paused_ptr))) ){
				if (p->freqratio==(1<<16)) { // 1/1
					S9xMixSamples((uint8*)dst,p->sample_count);
				} else if (p->freqratio==(1<<15)) { // 1/2
					S9xMixSamples((uint8*)src,p->sample_count>>1);
	 				for (i=p->sample_count>>2;i;i--) {
			 			*dst++=*src;
		 				*dst++=*src++;
				 	}
				} else if (p->freqratio==(1<<14)) { // 1/4
			 		S9xMixSamples((uint8*)src,p->sample_count>>2);
				 	for (i=p->sample_count>>3;i;i--) {
			 			*dst++=*src;
			 			*dst++=*src;
			 			*dst++=*src;
			 			*dst++=*src++;
				 	}
				} else { //generic case, missing some real interpolation
					int samples_count_corrected;
					samples_count_corrected=((p->sample_count*(p->freqratio)+(p->freqerr))>>16);
					S9xMixSamples((uint8*)src,samples_count_corrected);
					for (i=p->sample_count>>1;i;i--) {
						*dst++=*src;
						p->freqerr+=p->freqratio;
						if ((p->freqerr)&(1<<16)) {
			  			p->freqerr&=(1<<16)-1;
				 			src++;
			 			}
				 	}
				}
			} else { // paused or apu not enabled
				register int i,j;
				for (i=0;i<1024*1024;i++) j=j+1;
			}

			p->buffer_idx = next_buffer_idx;
			p->mixsample_flag=0;
		}

		//check if reset or load snapshot occured
		if (apu_init_after_load) {
			int intc = pspSdkDisableInterrupts();
    		if ((apu_init_after_load)&1) {
				S9xResetSound (TRUE);

				apu_ram_save=IAPU.RAM;
				memcpy(&IAPU,&IAPUuncached,sizeof(struct SIAPU));
				IAPU.RAM=apu_ram_save;
				memcpy(IAPU.RAM,IAPUuncached.RAM,0x10000);

				//memcpy(&APUPack.APU,&APUuncached,sizeof(struct SAPU));
				//do it each time since it's updatable from menu
				for (i = 0; i < 256; i++) S9xAPUCycles [i] = (int)S9xAPUCycleLengths [i] * (int)(IAPU.OneCycle) *os9x_apu_ratio / 256;

				memcpy(&APURegisters,&APURegistersUncached,sizeof(struct SAPURegisters));
				IAPU.DirectPage+=(IAPU.RAM)-(IAPUuncached.RAM);
  				IAPU.PC+=(IAPU.RAM)-(IAPUuncached.RAM);

	    		S9xSetEchoEnable (0);
    		}
			else if ((apu_init_after_load)&2) { //R5 - previous just IF
				//memcpy(&SoundData,(void*)SoundDataPtr,sizeof(SSoundData));
				(IAPU.PC) = (IAPU.RAM) + (APURegisters.PC);

				S9xAPUUnpackStatus ();
				if (APUCheckDirectPage ()) (IAPU.DirectPage) = (IAPU.RAM) + 0x100;
				else(IAPU.DirectPage) = (IAPU.RAM);
				S9xFixSoundAfterSnapshotLoad ();
				if ((*(p->os9x_apuenabled_ptr)==2)) S9xSetSoundMute( false );
			}
			apu_init_after_load=0;
			pspSdkEnableInterrupts( intc );
		}

		APU_EXECUTE3 ();

		if (p->exit) break;
	}
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
//uint8 apu_ram[0x10000];
void me_StartSound (){
	me_sound_t *p=(me_sound_t *)((int)(&me_sound_data)|0x40000000);
	//me_sceKernelDcacheWritebackInvalidateAll();
	CallME(me_data, (int)(&me_Dummy), 0, -1, 0, 0, 0); // all we want is the ME to inv its dcache
	sceKernelDcacheWritebackInvalidateAll();
	p->buffer[0]=(uint8 *)( ((int)SoundBuffer[0])|0x40000000 );
	p->buffer[1]=(uint8 *)( ((int)SoundBuffer[1])|0x40000000 );
	p->buffer[2]=(uint8 *)( ((int)SoundBuffer[2])|0x40000000 );
	p->buffer[3]=(uint8 *)( ((int)SoundBuffer[3])|0x40000000 );
	p->buffer_idx=2;
	p->sample_count=((stSoundStatus.buffer_size))>>1;
	p->freqratio=snd_freqratio;
	p->os9x_apuenabled_ptr=(int*)( ((int)&os9x_apuenabled)|0x40000000);
	p->os9x_paused_ptr=(int*)( ((int)&os9x_paused)|0x40000000);
	p->exit=0;
	//p->apu_ram=apu_ram;

	//me_start(me_data, (int)(&me_MixSound), (int)p);
	BeginME(me_data, (int)(&me_MixSound), (int)p, 0, 0, 0, 0);
}
#endif

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int S9xProcessSound (SceSize ,void *) {
	int i;
	uint8 *apu_ram_save;
	u32 sample_count=(stSoundStatus.buffer_size)>>1;
//debug_log( "Thread start!" );
	memset((char*)SoundBuffer[0],0,sample_count*2);
	memset((char*)SoundBuffer[1],0,sample_count*2);


	apu_ram_save=IAPU.RAM;
	memcpy(&IAPU,&IAPUuncached,sizeof(struct SIAPU));
	IAPU.RAM=apu_ram_save;
	memcpy(IAPU.RAM,IAPUuncached.RAM,0x10000);

//	memcpy(&APUPack.APU,&APUuncached,sizeof(struct SAPU));
	//do it each time since it's updatable from menu
	for (i = 0; i < 256; i++) S9xAPUCycles [i] = (int)S9xAPUCycleLengths [i] * (int)(IAPU.OneCycle) *os9x_apu_ratio / 256;

	memcpy(&APURegisters,&APURegistersUncached,sizeof(struct SAPURegisters));

	IAPU.DirectPage+=(IAPU.RAM)-(IAPUuncached.RAM);
	IAPU.PC+=(IAPU.RAM)-(IAPUuncached.RAM);

#ifdef ME_SOUND
	me_StartSound();
	me_sound_t *p=(me_sound_t *)((int)(&me_sound_data)|0x40000000);
	//sceKernelDelayThread(1000);
	do {
		if (!Settings.ThreadSound) break;
		//play buffer
		i=p->buffer_idx;
		p->mixsample_flag=1;

		if ((os9x_apuenabled==2)&&(!os9x_paused)) {
			sceAudioOutputPannedBlocking( (stSoundStatus.sound_fd), MAXVOLUME, MAXVOLUME, (char*)((int)SoundBuffer[i]|0x00000000));
		} else {
			// [Shoey] Uncommented next 2 lines
			//memset((char*)SoundBuffer[current_SoundBuffer],0,sample_count*2);
			//sceAudioOutputPannedBlocking( (so->sound_fd), MAXVOLUME, MAXVOLUME, (char*)((int)SoundBuffer[i]|0x00000000));
			sceKernelDelayThread(200*1000);//200ms wait
		}
	} while (Settings.ThreadSound);
	p->exit=1;
	WaitME(me_data);
#else
	do {
		int i;
		u32 *src,*dst;

		//check if reset or load snapshot occured
		if (apu_init_after_load) {
			if ((apu_init_after_load)&1) {
				S9xResetSound (TRUE);

				apu_ram_save=IAPU.RAM;
				memcpy(&IAPU,&IAPUuncached,sizeof(struct SIAPU));
				IAPU.RAM=apu_ram_save;
				memcpy(IAPU.RAM,IAPUuncached.RAM,0x10000);

//				memcpy(&APUPack.APU,&APUuncached,sizeof(struct SAPU));
				//do it each time since it's updatable from menu
				for (i = 0; i < 256; i++) S9xAPUCycles [i] = (int)S9xAPUCycleLengths [i] * (int)(IAPU.OneCycle) *os9x_apu_ratio / 256;

				memcpy(&APURegisters,&APURegistersUncached,sizeof(struct SAPURegisters));
				IAPU.DirectPage+=(IAPU.RAM)-(IAPUuncached.RAM);
  			IAPU.PC+=(IAPU.RAM)-(IAPUuncached.RAM);

    		S9xSetEchoEnable (0);
    	}
			else if ((apu_init_after_load)&2) {
				(IAPU.PC) = (IAPU.RAM) + (APURegisters.PC);

				S9xAPUUnpackStatus ();
				if (APUCheckDirectPage ()) (IAPU.DirectPage) = (IAPU.RAM) + 0x100;
				else(IAPU.DirectPage) = (IAPU.RAM);
				S9xFixSoundAfterSnapshotLoad ();
				if (os9x_apuenabled==2) S9xSetSoundMute( false );
			}
			apu_init_after_load=0;
		}

		if ((os9x_apuenabled==2)&&(!os9x_paused)&&((apu_init_after_load)==0) ) {
			if (snd_freqratio==(1<<16)) { // 1/1
	  			S9xMixSamples((uint8*)SoundBuffer[current_SoundBuffer],sample_count);//stereo & 16bits
	  		} else if (snd_freqratio==(1<<15)) { // 1/2
	  			S9xMixSamples((uint8*)SoundBuffer[2],sample_count>>1);//stereo & 16bits
				src=(u32*)SoundBuffer[2];
	  			dst=(u32*)SoundBuffer[current_SoundBuffer];
	  			for (i=sample_count>>2;i;i--) {
	  				*dst++=*src;
	  				*dst++=*src++;
	  			}
	  		} else { //generic case, missing some real interpolation
	  			S9xMixSamples((uint8*)SoundBuffer[2],(sample_count*snd_freqratio)>>16);//stereo & 16bits
				src=(u32*)SoundBuffer[2];
	  			dst=(u32*)SoundBuffer[current_SoundBuffer];
	  			for (i=sample_count>>1;i;i--) {
	  				*dst++=*src;
	  				snd_freqerr+=snd_freqratio;
	  				if (snd_freqerr&(1<<16)) {
		  				snd_freqerr&=(1<<16)-1;
	  					src++;
	  				}
	  			}
			}
	  		i=current_SoundBuffer;
  			sceAudioOutputPannedBlocking( (stSoundStatus.sound_fd), MAXVOLUME, MAXVOLUME, (char*)SoundBuffer[i]);
  			current_SoundBuffer^=1;
		} else {
  			//memset((char*)SoundBuffer[current_SoundBuffer],0,sample_count*2);
  			//sceAudioOutputPannedBlocking( (so->sound_fd), MAXVOLUME, MAXVOLUME, (char*)SoundBuffer[i]);
  			sceKernelDelayThread(200*1000);//200ms wait
  		}

  	//i=current_SoundBuffer;
  	//sceAudioOutputPannedBlocking( (so->sound_fd), MAXVOLUME, MAXVOLUME, (char*)SoundBuffer[i]);
  	//current_SoundBuffer^=1;

  } while (Settings.ThreadSound);



#endif


	(APURegisters.PC) = (IAPU.PC) - (IAPU.RAM);
	S9xAPUPackStatus ();
	memcpy(&APURegistersUncached,&APURegisters,sizeof(struct SAPURegisters));

	apu_ram_save=IAPUuncached.RAM;
	memcpy(&IAPUuncached,&IAPU,sizeof(struct SIAPU));
	IAPUuncached.RAM=apu_ram_save;
	memcpy(IAPUuncached.RAM,IAPU.RAM,0x10000);

	IAPUuncached.DirectPage+=(IAPUuncached.RAM)-(IAPU.RAM);
	IAPUuncached.PC+=(IAPUuncached.RAM)-(IAPU.RAM);


	//*((int*)(APUPack.APU.OutPorts))=*((int*)(APUPack.APU.OutPorts));//to avoid losing OutPorts value
//	memcpy(&APUuncached,&APUPack.APU,sizeof(struct SAPU));
	//	debug_log( "thread end");
  return (0);
}
// main->call
void S9xSuspendSoundProcess(void)
{
#ifdef ME_SOUND
	/*
	volatile PROCESS_EVENT *pEvent = (PROCESS_EVENT *)UNCACHE_PTR(&g_stProcessEvent);
	pEvent->abQueueData[pEvent->bMainQueuePtr] = EVENT_SUSPEND;
	pEvent->bMainQueuePtr = (pEvent->bMainQueuePtr + 1) &QUEUE_MASK;
	if (pEvent->bMeWorking) {
		// waits until me starts suspending.
		while (!pEvent->wSuspend) {
			{
				pgPrintBG(0,7,0xFFFF,"maybe deadlock");
				pgScreenFlipV();
			}
			;
		}
	}
*/
#endif
}

// main->call
void S9xResumeSoundProcess(void)
{
#ifdef ME_SOUND
	/*
	sceKernelDcacheWritebackInvalidateAll();
	PROCESS_EVENT *pEvent = (PROCESS_EVENT *)UNCACHE_PTR(&g_stProcessEvent);
	pEvent->abQueueData[pEvent->bMainQueuePtr] = EVENT_RESUME;
	pEvent->bMainQueuePtr = (pEvent->bMainQueuePtr + 1) &QUEUE_MASK;
	*/
#endif
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void InitSoundThread(){
//	if (os9x_apuenabled<2) return;
	if (g_sndthread!=-1) return;
	//debug_log( "Create Thread" );
	//me_startproc((u32)me_function, (u32)me_data); // [jonny]

	g_sndthread = sceKernelCreateThread( "sound thread", (SceKernelThreadEntry)S9xProcessSound, 0x8, 256*1024, 0, 0 );
	if ( g_sndthread < 0 ){
		ErrorExit( "Thread failed" );
		return;
	}
	Settings.SoundPlaybackRate = os9x_sndfreq;
	snd_freqratio = (u32)(Settings.SoundPlaybackRate)*(1<<16) / 44100;
	snd_freqerr=0;
	samples_error=0;
	current_SoundBuffer=0;
	S9xSetPlaybackRate( Settings.SoundPlaybackRate  );
//APUPack.APU.Cycles=10;
//	volatile int32 *cycles;\ 
//	cycles=&(APUPack.APU.Cycles);
	Settings.ThreadSound = true;
	sceKernelStartThread( g_sndthread, 0, 0 );

	sceKernelDelayThread(100*1000);
	while (apu_init_after_load) {
		sceKernelDelayThread(100*1000);
	}
	//	int r=0;// = 	WaitME(me_data);
	//while(*cycles==10){		sceKernelDelayThread(100*1000);}
	//char temp[200]; 
	//sprintf(temp,"S9xProcessSound ret=%d,val=%d val2=%d",*cycles,APUPack.APU.Cycles,Uncache_APU_Cycles);
	//ErrorExit(temp);
	//return ;

	//debug_log( "Thread ok" );
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void StopSoundThread(){
	//if (os9x_apuenabled<2) return; //not  needed since g_sndthread will be -1
	if ( g_sndthread !=-1 ){
		Settings.ThreadSound = false;
		sceKernelWaitThreadEnd( g_sndthread, NULL );
		sceKernelDeleteThread( g_sndthread );
		//me_stopproc();
		g_sndthread=-1;
	}
}


////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void S9xSetPalette(){
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void S9xMessage( int type, int number, const char* message ){
	msgBoxLines(message,30);
	//debug_log( message );
	//S9xSetInfoString( message );
	if ((type==-1)||(type==S9X_ERROR)) pgwaitPress();
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
bool8 S9xReadSuperScopePosition (int &x, int &y, uint32 &buttons)
{
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
bool8 S9xReadMousePosition (int which, int &x, int &y, uint32 &buttons)
{
	return FALSE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
uint32 S9xReadJoypad( int which1 ) {
	int val;

	if (os9x_netplay) {
		if (which1<5) val=os9x_netsnespad[0][which1];
		else val=0;
	} else {
		if ( which1 == os9x_padindex) val=os9x_snespad;
		else val=0;
	}
	return val | 0x80000000;
}



////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void S9xSyncSpeed()
{
	static int waited=1;
	struct timeval now;

	s_TotalFrame++;

	os9x_updatepadcpt++;

	if (os9x_netplay) {
		if (os9x_updatepadcpt>=NET_UPDATE_INTERVAL) {
			os9x_updatepadcpt=0;
			os9x_updatepadFrame++;
			update_pad(); //1 frame / 4
		}
	} else {
		if (os9x_updatepadcpt>=OFFLINE_UPDATE_INTERVAL) {
			update_pad(); //1 frame / 2
			os9x_updatepadcpt=0;
		}
	}

	s_iFrame++;
	s_iFrameAuto++;

	S9xProcessEvents( FALSE );

	sceKernelLibcGettimeofday( &now, 0 );
	if ( next1.tv_sec == 0 ){
		next1 = now;
		++next1.tv_usec;
	}
	if ( next1_autofs.tv_sec == 0 ){
		next1_autofs = now;
		++next1_autofs.tv_usec;
	}

	if ( os9x_TurboMode ){
		if( ++IPPU.FrameSkip >= Settings.TurboSkipFrames ){
			IPPU.FrameSkip = 0;
			IPPU.SkippedFrames = 0;
			IPPU.RenderThisFrame = TRUE;
		} else {
			++IPPU.SkippedFrames;
			IPPU.RenderThisFrame = FALSE;
		}
		return;
	}

	if (os9x_speedlimit){
		waited=0;

		if (IPPU.RenderThisFrame) {//if we have draw a frame, sync speed
		  if ( timercmp( &next1, &now, < ) ){
		  	//too slow
				unsigned int lag;
				/*lag = (now.tv_sec - next1.tv_sec) * 1000000 + now.tv_usec - next1.tv_usec;
				if ( lag >= 1000000*1 ){ //1s lag => reset
					next1 = now;
				} else if (lag <= MIN_AUTOFRAME_LAG) waited=1;*/
				//reset time
				next1 = now;
		  } else {
		  	//too fast,
		  	//wait to sync
		  	waited = (now.tv_sec - next1.tv_sec) * 1000000 + now.tv_usec - next1.tv_usec;
		  	while ( timercmp( &next1, &now, > ) ){
					sceKernelLibcGettimeofday( &now, 0 );
		  	}
		  }
		}

	  // update next timer value
	  if (!os9x_fpslimit) {
	  	next1.tv_usec += ((Settings.PAL?1000000/50:1000000/60));
	  } else {
	  	next1.tv_usec += 1000000/os9x_fpslimit;
	  }
	  while ( next1.tv_usec >= 1000000 ){
	  	next1.tv_sec += 1;
      next1.tv_usec -= 1000000;
	  }
	}

	if (Settings.SkipFrames==AUTO_FRAMERATE) {
		//AUTO FRAME SKIPPING
		if (IPPU.RenderThisFrame) {
			//WE HAVE RENDERED A FRAME, so sync was performed
	  	if (!waited && (os9x_autofskip_SkipFrames<MAX_AUTO_SKIP) ) {
	  		//it was too slow
				os9x_autofskip_SkipFrames++;
			} else {
				//it was too fast
				//perhaps add a limit to wait => if (wait>LIMIT) os9x_autofskip_SkipFrames--...
				if (os9x_autofskip_SkipFrames) os9x_autofskip_SkipFrames--;
			}
		}
		IPPU.RenderThisFrame = ++IPPU.SkippedFrames > os9x_autofskip_SkipFrames;
		if ( IPPU.RenderThisFrame ) IPPU.SkippedFrames = 0;
	} else {
		IPPU.RenderThisFrame = ++IPPU.SkippedFrames > Settings.SkipFrames;
		if ( IPPU.RenderThisFrame ) IPPU.SkippedFrames = 0;
	}


}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
const char *S9xGetFilename( const char *e )
{
	static char filename [_MAX_PATH + 1];
	char drive [_MAX_DRIVE + 1];
	char dir [_MAX_DIR + 1];
	char fname [_MAX_FNAME + 1];
	char ext [_MAX_EXT + 1];

	_splitpath (Memory.ROMFilename, drive, dir, fname, ext);
	_makepath (filename, drive, dir, fname, e);

	return (filename);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
const char *S9xGetSaveFilename( const char *e ) {
	static char filename [_MAX_PATH + 1];
	char drive [_MAX_DRIVE + 1];
	char dir [_MAX_DIR + 1];
	char fname [_MAX_FNAME + 1];
	char ext [_MAX_EXT + 1];

	_splitpath (Memory.ROMFilename, drive, dir, fname, ext);
	sprintf(dir,"%sSAVES",LaunchDir);
	_makepath (filename, drive, dir, fname, e);

	return (filename);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
bool8 S9xInitUpdate() {
	os9x_renderingpass=0;

	if (os9x_softrendering>=2) {//GU mode
		sceGuStart(0,list);

		/* setup view port */
		//sceGuOffset(2048 - (256/2),2048 - (240/2));
		//sceGuViewport(2048,2048,256,240);
		sceGuDrawBufferList(GU_PSM_5551,(void*)(512*272*2*2+256*240*2+2*256*256*2),256);

		/*clear screen */
		sceGuScissor(0,0,256,os9x_snesheight);
		sceGuClearColor(0);
		//sceGuClearDepth(0);
		sceGuClear(GU_COLOR_BUFFER_BIT/*|GU_DEPTH_BUFFER_BIT*/);

		sceGuFinish();
		sceGuSync(0,0);
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void resync_var(){

    if (os9x_fskipvalue==AUTO_FSKIP) {
    	Settings.SkipFrames=AUTO_FRAMERATE;
    	os9x_autofskip_SkipFrames=0;
    	os9x_speedlimit=1;
    } else Settings.SkipFrames=os9x_fskipvalue;

    //uncaching stuff
    int tmp=os9x_apuenabled;
    *os9x_apuenabled_ptr=tmp;
		Settings.NextAPUEnabled = Settings.APUEnabled = (os9x_apuenabled==1)||(os9x_apuenabled==2);

		/*if ((!os9x_apuenabled)||(os9x_apuenabled==3))*/ os9x_hack|=APU_FIX;

		os9x_snesheight=(Settings.PAL&&(!os9x_forcepal_ntsc)?240:224);

		set_cpu_clock();

		sceKernelLibcGettimeofday( &s_tvStart, 0 );
		os9x_autosavetimer_tv=s_tvStart;

		//reset timer for synchro stuff
		next1.tv_sec = next1.tv_usec = 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void before_pause(){
	//os9x_paused=1;
	*os9x_paused_ptr=1;
	StopSoundThread();
#ifndef ME_SOUND
	scePowerSetClockFrequency(222,222,111);
#endif
	//scePowerSetClockFrequency(300,300,150);
	Settings.Paused = TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void after_pause(){
	pgFillAllvram(0);
	Settings.Paused = false;
	//os9x_paused=0;
	*os9x_paused_ptr=0;
	//reinit blitter
	blit_reinit();
	//invalidate all gfx caches
	ZeroMemory (IPPU.TileCached [TILE_2BIT], MAX_2BIT_TILES<<1);
  ZeroMemory (IPPU.TileCached [TILE_4BIT], MAX_4BIT_TILES<<1);
  ZeroMemory (IPPU.TileCached [TILE_8BIT], MAX_8BIT_TILES<<1);
  tile_askforreset(-1);
  //
	resync_var();
	InitSoundThread();
	if (os9x_apuenabled==2)	S9xSetSoundMute( false );
	else S9xSetSoundMute( true );
}
////////////////////////////////////////////////////////////////////////////////////////
// Ge Callback
////////////////////////////////////////////////////////////////////////////////////////
struct timeval	now;
	unsigned long long	diff;
	static int fps_val=0;
	static int real_fps_val=0;
int g_cbid;
#define pg_vramtop ((char *)0x04000000)
// Ge render callback
void GeCallback(int id, void *arg)
{
	char			buf[128];

	swap_buf^=1;
	pg_drawframe=swap_buf^1;

	if (os9x_showpass){
		sprintf(buf,"%03d",os9x_renderingpass);
		pgPrintBG(CMAX_X-8-strlen(buf),0,0xffff,buf);

#ifndef	RELEASE
		int Y=1;
		char str[64];
		#define LOG_PROFILE_FUNC(func,type) \
			sprintf(str,"%25s (...) =%u usecs\n", #func, type.time_##func); \
	    pgPrintBG (0,Y++,0xFFFF,str);

			LOG_PROFILE_FUNC (S9xMainLoop, profile_data)
			RESET_PROFILE_FUNC(S9xMainLoop)
#endif
	}
	os9x_renderingpass=0;


	if (os9x_TurboMode) {
		if ((s_TotalFrame>>6)&1) pgPrintBG(CMAX_X-5,33,0xffff,"TURBO");
		else pgPrintBG(CMAX_X-5,33,0xffff,"     ");
	}

	if (os9x_lowbat) {
		if (!((s_TotalFrame>>7)&15)) pgPrintBG(0,33,0xffff,"Low Battery/Saving disactivated");
		else pgPrintBGRev(0,33,0xffff,"                               ");
	}

	s_iFrameReal++;
	sceKernelLibcGettimeofday( &now, 0 );
	if (os9x_showfps) {
		diff  = (now.tv_sec - s_tvStart.tv_sec) * 1000000 + now.tv_usec - s_tvStart.tv_usec;
		diff /= 1000000;
		if ( diff>=2 ) {
				fps_val = s_iFrame/diff;
				real_fps_val = s_iFrameReal/diff;
				s_tvStart = now;
				s_iFrame  = 0;
				s_iFrameReal=0;
		}

		if (fps_val) {
			buf[0] = ((fps_val / 100)%10) + '0';
			buf[1] = ((fps_val / 10)%10) + '0';
			buf[2] = (fps_val % 10) + '0';
			buf[3] = 'F';
			buf[4] = 'P';
			buf[5] = 'S';
			buf[6] = '\0';
			pgPrintBG( CMAX_X - 7, 0, 0xffff, buf );
		}
		/*
		if (real_fps_val) {
			buf[0] = ((real_fps_val / 100)%10) + '0';
			buf[1] = ((real_fps_val / 10)%10) + '0';
			buf[2] = (real_fps_val % 10) + '0';
			pgPrintBG( CMAX_X - 7, 1, 0xffff, buf );
		}
		*/
	}
	//MyCounter_drawCount();
/*	{
		char st[108];
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//		volatile PROCESS_EVENT *pEvent = (PROCESS_EVENT *)UNCACHE_PTR(&g_stProcessEvent);
//		sprintf(st,"%02X, %02X, %02X, %02X, %02X",
//			pEvent->wCallCount, pEvent->wSuspend, pEvent->bMeWorking, pEvent->bMeQueuePtr, pEvent->bMainQueuePtr);
//		pgPrintBG(0,2,0xFFFF,st);
		SAPUEVENTS *pEvent2 = (SAPUEVENTS *)UNCACHE_PTR(&stAPUEvents);\
		sprintf(st,"E1[%04X,%04X] E2[%04X,%04X], RW[%04X,%04X]",
			pEvent2->apu_event1_cpt1&0xFFFF, pEvent2->apu_event1_cpt2&0xFFFF,
			pEvent2->apu_event2_cpt1&0xFFFF, pEvent2->apu_event2_cpt2&0xFFFF,
			pEvent2->apu_ram_write_cpt1&0xFFFF, pEvent2->apu_ram_write_cpt2&0xFFFF);
		pgPrintBG(0,2,0xFFFF,st);
		sprintf(st,"%08X, %08X, %08X",
			pEvent2->APU_Cycles, pEvent2->apu_glob_cycles, cpu_glob_cycles);
		pgPrintBG(0,3,0xFFFF,st);
		sprintf(st,"[%08X,%08X,%08X,%08X]",
			pEvent2->adwParam[0], pEvent2->adwParam[1], pEvent2->adwParam[2], pEvent2->adwParam[3]);
		pgPrintBG(0,4,0xFFFF,st);
	}*/

	/*{
		char st[32];
		sprintf(st,"%08X",os9x_updatepadFrame);
		pgPrintBG(CMAX_X-10,7,0xFFFF,st);
	}*/

	sceDisplaySetFrameBuf(pg_vramtop+(pg_drawframe?FRAMESIZE:0),LINESIZE,1,
		os9x_vsync ? PSP_DISPLAY_SETBUF_NEXTFRAME: PSP_DISPLAY_SETBUF_IMMEDIATE);
}

void SetGeCallback(void)
{
	PspGeCallbackData cb;

	cb.signal_func = NULL;
	cb.signal_arg = NULL;
	cb.finish_func = GeCallback;
	cb.finish_arg = NULL;
	g_cbid = sceGeSetCallback(&cb);

}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
bool8 S9xDeinitUpdate (int Width, int Height, bool8 sixteen_bit) {
	char			buf[128];


	//if (os9x_hack & HIRES_FIX) {
	//	hires_swap++;
	//	if (hires_swap>=3) hires_swap=0;
	//	switch (hires_swap) {
	//		case 0:hires_offset=0;hires_offset2=0;hires_offsetV=0;hires_offsetV2=0;
	//			break;
	//		case 1:hires_offset=1;hires_offset2=0;hires_offsetV=1;hires_offsetV2=0;
	//			break;
	//		case 2:hires_offset=0;hires_offset2=1;hires_offsetV=0;hires_offsetV2=1;
	//			break;
	//	}
	//}

	if (os9x_softrendering>=2) {
		switch (os9x_render) {
			case 0:
  					//could be faster since no stretch, use copyimage
  					guDrawBuffer((u16*)(0x44000000+512*272*2*2+256*240*2+2*256*256*2),256,os9x_snesheight,256,256,os9x_snesheight);
  					break;
			case 1:
					guDrawBuffer((u16*)(0x44000000+512*272*2*2+256*240*2+2*256*256*2),256,os9x_snesheight,256,256*272/os9x_snesheight,272);
				break;
			case 2:
					guDrawBuffer((u16*)(0x44000000+512*272*2*2+256*240*2+2*256*256*2),256,os9x_snesheight,256,272*4/3,272);
					//pgBitBltFull((unsigned long*)(0x44000000+512*272*2*2+256*240*2+2*256*256*2),os9x_snesheight,0,272);
				break;
			case 3:
					guDrawBuffer((u16*)(0x44000000+512*272*2*2+256*240*2+2*256*256*2),256,os9x_snesheight,256,320*272/os9x_snesheight,272);
				break;
			case 4:
					guDrawBuffer((u16*)(0x44000000+512*272*2*2+256*240*2+2*256*256*2),256,os9x_snesheight,256,480,272);
				break;
			case 5:
					guDrawBuffer((u16*)(0x44000000+512*272*2*2+256*240*2+2*256*256*2),256,os9x_snesheight-16,256,480,272);
				break;
		}
  } else {
		switch (os9x_render) {
			case 0:
				//could be faster since no stretch, use copyimage
				guDrawBuffer((u16*)(GFX.Screen),256,os9x_snesheight,256,256,os9x_snesheight);
				break;
			case 1:	guDrawBuffer((u16*)(GFX.Screen),256,os9x_snesheight,256,256*272/os9x_snesheight,272);
				break;
			case 2:	guDrawBuffer((u16*)(GFX.Screen),256,os9x_snesheight,256,272*4/3,272);
				break;
			case 3:	guDrawBuffer((u16*)(GFX.Screen),256,os9x_snesheight,256,320*272/os9x_snesheight,272);
				break;
			case 4:	guDrawBuffer((u16*)(GFX.Screen),256,os9x_snesheight,256,480,272);
				break;
			case 5:	guDrawBuffer((u16*)(GFX.Screen),256,os9x_snesheight-16,256,480,272);
				break;
		}
	}

	return TRUE;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void S9xInitDisplay(  )
{
//	Settings.Transparency = TRUE;
	Settings.SixteenBit   = TRUE;
	Settings.SupportHiRes = 0; //interpolate;
	memset( GFX_SubScreen,  0, SNES_WIDTH * SNES_HEIGHT_EXTENDED * 2 );
	memset( GFX_ZBuffer,    0, SNES_WIDTH * SNES_HEIGHT_EXTENDED  );
	memset( GFX_SubZBuffer, 0, SNES_WIDTH * SNES_HEIGHT_EXTENDED  );
	GFX.Pitch      = 256 * 2;
	//screen & zbuffer share the same memory location since they should not be used
	//at the same time for the same location
  GFX.Screen   = (uint8*)(0x44000000+512*272*2*2);
  GFX.SubScreen  = (uint8*)GFX_SubScreen;
	GFX.ZBuffer    = (uint8*)GFX_ZBuffer;
	GFX.SubZBuffer = (uint8*)GFX_SubZBuffer;
}




////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void S9xProcessEvents( bool8 block ) {
	static int cpt=0;
	if (!(cpt&2047)) scePowerTick(0); //send powertick to avoid auto off
	if (!(cpt&2047)) {//check every 34s
		//check if battery is low
		check_battery();
	}
	cpt++;
	if (os9x_specialaction&OS9X_MENUACCESS) {
		if ((in_emu==1)&&os9x_netplay) {		//net pause
			net_flush_net(2);
		}
		before_pause();
		if (!os9x_lowbat) {
			if (CPUPack.CPU.SRAMModified) {
				Memory.SaveSRAM( (char*)S9xGetSaveFilename(".SRM") );
				CPUPack.CPU.SRAMModified=0;
			}
		}
		//initUSBdrivers();

		root_menu();

#ifndef FW3X
		endUSBdrivers();
#endif
		os9x_specialaction=0;
		if (!os9x_lowbat) {
			if (menu_modified) {
				save_rom_settings(Memory.ROMCRC32,Memory.ROMName);
				save_settings();
			}
		}
		if ((in_emu==1)&&os9x_netplay&&os9x_getnewfile) {
			msgBoxLines("closing connection",60);
			os9x_netplay=0;os9x_adhoc_active=0;
		}

		if ((in_emu==1)&&os9x_netplay) {		//net unpause
			set_cpu_clock();
			net_send_state();
			net_send_settings();
		}
		after_pause();
	}
	if ((os9x_specialaction&OS9X_GFXENGINE)&& (!(os9x_specialaction_old&OS9X_GFXENGINE))) {
		os9x_softrendering++;
		if (os9x_softrendering==5) os9x_softrendering=0;
		//invalidate all cache
		ZeroMemory (IPPU.TileCached [TILE_2BIT], MAX_2BIT_TILES<<1);
  	ZeroMemory (IPPU.TileCached [TILE_4BIT], MAX_4BIT_TILES<<1);
  	ZeroMemory (IPPU.TileCached [TILE_8BIT], MAX_8BIT_TILES<<1);
  	tile_askforreset(-1);
  	//
		switch (os9x_softrendering) {
			case 4:msgBoxLines("Fast + approx. software",30);break;
			case 3:msgBoxLines("Fast accurate software",30);break;
			case 2:msgBoxLines("Fast",30);break;
			case 1:msgBoxLines("Accurate software",30);break;
			case 0:msgBoxLines("Approx. software",30);break;
		}
		//reset timer for synchro stuff
		next1.tv_sec = next1.tv_usec = 0;
  }
	if ((os9x_specialaction&OS9X_FRAMESKIP_DOWN)&& (!(os9x_specialaction_old&OS9X_FRAMESKIP_DOWN))) {
		char st[64];
		if (os9x_fskipvalue) os9x_fskipvalue--;
		else os9x_fskipvalue=AUTO_FSKIP;

		if (os9x_fskipvalue==AUTO_FSKIP) {
			Settings.SkipFrames=AUTO_FRAMERATE;
			os9x_autofskip_SkipFrames=0;
			os9x_speedlimit=1;
			sprintf(st,"Frameskip : AUTO");
		} else {
			Settings.SkipFrames=os9x_fskipvalue;
			sprintf(st,"Frameskip : %d",os9x_fskipvalue);
		}
		msgBoxLines(st,10);
		//reset timer for synchro stuff
		next1.tv_sec = next1.tv_usec = 0;
	}
	if ((os9x_specialaction&OS9X_FRAMESKIP_UP) && (!(os9x_specialaction_old&OS9X_FRAMESKIP_UP))) {
		char st[64];
		if (os9x_fskipvalue<AUTO_FSKIP) os9x_fskipvalue++;
		else os9x_fskipvalue=0;
		if (os9x_fskipvalue==AUTO_FSKIP) {
			Settings.SkipFrames=AUTO_FRAMERATE;
			os9x_speedlimit=1;
			os9x_autofskip_SkipFrames=0;
			sprintf(st,"Frameskip : AUTO");
		} else {
			Settings.SkipFrames=os9x_fskipvalue;
			sprintf(st,"Frameskip : %d",os9x_fskipvalue);
		}
		msgBoxLines(st,10);
		//reset timer for synchro stuff
		next1.tv_sec = next1.tv_usec = 0;
	}
	if ((os9x_specialaction&OS9X_TURBO)&& (!(os9x_specialaction_old&OS9X_TURBO))) {
		if (os9x_TurboMode) {
			pgPrintAllBG(CMAX_X-5,33,0xffff,"     ");
			//reset timer for synchro stuff
			next1.tv_sec = next1.tv_usec = 0;
		}
		os9x_TurboMode^=1;
	}
	if (os9x_autosavetimer) {
		struct timeval now;
		u32 diff;
		sceKernelLibcGettimeofday( &now, 0 );
		diff  = (now.tv_sec - os9x_autosavetimer_tv.tv_sec) * 1000000 + now.tv_usec - os9x_autosavetimer_tv.tv_usec;
		diff/=1000000;
		if ( diff>=60*os9x_autosavetimer ) {
			os9x_autosavetimer_tv=now;
			if (!os9x_lowbat) {
				msgBoxLines("Autosaving...",0);
				os9x_save(".zat");
				//reset timer for synchro stuff
				next1.tv_sec = next1.tv_usec = 0;
			}
		}
	}
	os9x_specialaction_old=os9x_specialaction;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void S9xExit (){
	ExitCallback(0,0,NULL);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
const char *S9xBasename (const char *f)
{
  const char *p;
  if ((p = strrchr (f, '/')) != NULL || (p = strrchr (f, '\\')) != NULL)
    return (p + 1);
  return (f);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
bool8 S9xOpenSnapshotFile (const char *fname, bool8 read_only, STREAM *file) {
  if (read_only) {
      if (!(*file = OPEN_STREAM(fname,"rb"))) return(false);
	} else {
    if (!(*file = OPEN_STREAM(fname,"wb"))) return(false);
	}
	char *ext;
	ext=strrchr(fname,'.');
	if (ext&&(strlen(ext)==4)) {
		if ((ext[1]=='z')&&(ext[2]=='a')) {
			if (os9x_externstate_mode) {
				msgBoxLines("Found a snes9xTYL file");
				os9x_externstate_mode=0;
			}
		}
  }


  if (os9x_externstate_mode) return true;

  if (read_only) {//reading savestate
		READ_STREAM(os9x_savestate_mini,128*120*2,*file);
	} else { //writing savestate
		int x,y;
		u16 *snes_image;
		if (os9x_softrendering<2)	snes_image=(u16*)(0x44000000+512*272*2*2);
		else snes_image=(u16*)(0x44000000+2*512*272*2+256*240*2+2*256*256*2);
		for (y=0;y<os9x_snesheight/2;y++)
			for (x=0;x<128;x++) {
				int col2a=snes_image[(y*2)*256+(x*2)];
				int col2b=snes_image[(y*2+1)*256+(x*2)];
				int col2c=snes_image[(y*2)*256+(x*2+1)];
				int col2d=snes_image[(y*2+1)*256+(x*2+1)];
				int col2;
				col2=((((((col2a>>10)&31)+((col2b>>10)&31)+((col2c>>10)&31)+((col2d>>10)&31))>>2)/**2/3*/)<<10);
				col2|=((((((col2a>>5)&31)+((col2b>>5)&31)+((col2c>>5)&31)+((col2d>>5)&31))>>2)/**2/3*/)<<5);
				col2|=((((((col2a>>0)&31)+((col2b>>0)&31)+((col2c>>0)&31)+((col2d>>0)&31))>>2)/**2/3*/)<<0);
				os9x_savestate_mini[y*128+x]=col2;
			}
		WRITE_STREAM(os9x_savestate_mini,128*120*2,*file);
	}
  return (true);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void S9xCloseSnapshotFile (STREAM file) {
  CLOSE_STREAM (file);
}

extern "C" {

#include "psp_state.c"

#include "psp_utils.c"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void initvar_withdefault() {
	bg_img_num=-1; //bg will be randomized

	os9x_menumusic=0; //no music
	os9x_menufx=0; //menu background FX disabled
	os9x_menupadbeep=1; //beep when selecting something
	os9x_usballowed=0; //no usb

	os9x_apu_ratio=256; //100%
	os9x_fpslimit=0; //AUTO
	os9x_padindex=0;

	g_bSleep=0;
	//////
	os9x_padvalue=0;
	os9x_padvalue_ax=0x80;
	os9x_padvalue_ay=0x80;
	//
	os9x_screenLeft=os9x_screenTop=os9x_screenWidth=os9x_screenHeight=0;

	os9x_autosavetimer=0;
	os9x_autosavesram=0;
	os9x_lowbat=0;
	check_battery();
	//if (scePowerIsBatteryExist()) os9x_lowbat=scePowerIsLowBattery();
	os9x_applyhacks=1;
	os9x_BG0=1;
	os9x_BG1=1;
	os9x_BG2=1;
	os9x_BG3=1;
	os9x_OBJ=1;
	os9x_easy=0;
	os9x_render=2;  //zoom 4/3 (tv mode)
	os9x_showfps=0;
	os9x_showpass=0;
	os9x_vsync=0;
	os9x_cpuclock=333;

	os9x_apuenabled=2;

	os9x_gammavalue=0;
	os9x_fastsprite=0;
	os9x_softrendering=2;//psp accel (Fast)
	os9x_smoothing=1;
	os9x_fskipvalue=0;
	os9x_autofskip_SkipFrames=0;
	os9x_speedlimit=1;

	os9x_forcepal_ntsc=1; //most pal games have black bottom borders
	os9x_sndfreq = 11025;
	/** not in menu at the moment **/
	os9x_ShowSub=0;
	os9x_CyclesPercentage=100;
	os9x_DisableHDMA=0;
	os9x_DisableIRQ=0;

	// special hack
	os9x_hack=0;

	//default inputs
	os9x_inputs_analog=0;
	memset(os9x_inputs,0,sizeof(os9x_inputs));
	os9x_inputs[PSP_UP]=SNES_UP_MASK;
	os9x_inputs[PSP_DOWN]=SNES_DOWN_MASK;
	os9x_inputs[PSP_LEFT]=SNES_LEFT_MASK;
	os9x_inputs[PSP_RIGHT]=SNES_RIGHT_MASK;
	os9x_inputs[PSP_START]=SNES_START_MASK;
	os9x_inputs[PSP_SELECT]=SNES_SELECT_MASK;
	os9x_inputs[PSP_CIRCLE]=SNES_A_MASK;
	os9x_inputs[PSP_CROSS]=SNES_B_MASK;
	os9x_inputs[PSP_SQUARE]=SNES_Y_MASK;
	os9x_inputs[PSP_TRIANGLE]=SNES_X_MASK;
	os9x_inputs[PSP_TL]=SNES_TL_MASK;
	os9x_inputs[PSP_TR]=SNES_TR_MASK;
	/*os9x_inputs[PSP_AUP]=OS9X_FRAMESKIP_UP;
	os9x_inputs[PSP_ADOWN]=OS9X_FRAMESKIP_DOWN;
	os9x_inputs[PSP_ALEFT]=OS9X_MENUACCESS;
	os9x_inputs[PSP_ARIGHT]=OS9X_TURBO;*/
#ifndef HOME_HOOK
  os9x_inputs[PSP_TL_TR]=OS9X_MENUACCESS; // [Shoey]
#endif
	os9x_inputs[PSP_TL_SELECT]=OS9X_TURBO;
	os9x_inputs[PSP_TL_START]=OS9X_GFXENGINE;
	os9x_inputs[PSP_TR_SELECT]=OS9X_FRAMESKIP_UP;
	os9x_inputs[PSP_TR_START]=OS9X_FRAMESKIP_DOWN;
	//os9x_inputs[PSP_NOTE]=OS9X_GFXENGINE;

	os9x_specialaction=os9x_specialaction_old=0;

	//
}







#ifndef NOKERNEL
void MyExceptionHandler(PspDebugRegBlock *regs)
{
	// Do normal initial dump, setup screen etc
	pspDebugScreenInit();

	// I always felt BSODs were more interesting that white on black
	pspDebugScreenSetBackColor(0x00FF0000);
	pspDebugScreenSetTextColor(0xFFFFFFFF);
	pspDebugScreenClear();

	pspDebugScreenPrintf("I regret to inform you your psp has just crashed\n");
	pspDebugScreenPrintf("\nShit happens... :-(\n");
	pspDebugScreenPrintf("Exception Details:\n");
	pspDebugDumpException(regs);
	pspDebugScreenPrintf("\nWill exit to PSP menu in 10 seconds\n");

	pgWaitVn(60*10);

	sceKernelExitGame();
}
#endif



void intro_anim();

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int user_main(SceSize args, void* argp);


////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int main(int argc,char **argv) {
	// Kernel mode thread
	/* Install our custom exception handler. If this was NULL then the default would be used */
#ifndef NOKERNEL
	pspDebugInstallErrorHandler(MyExceptionHandler);
#endif
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(0);
	/************/
  //pspDebugScreenInit();
  pgFillAllvram(0);
	sceDisplaySetMode( 0, SCREEN_WIDTH, SCREEN_HEIGHT );
	sceDisplaySetFrameBuf( (char*)VRAM_ADDR, 512, 1, 1 );

	pgScreenFrame(2,0);
	pgFillAllvram(0);

	strncpy(LaunchDir,argv[0],sizeof(LaunchDir)-1);
	LaunchDir[sizeof(LaunchDir)-1]=0;
	char *str_ptr=strrchr(LaunchDir,'/');
	if (str_ptr){
		str_ptr++;
		*str_ptr=0;
	}
  /* Clear the existing profile regs */
	//pspDebugProfilerClear();
	/* Enable profiling */
	//pspDebugProfilerEnable();
	char str[256];
	int devkit_version = sceKernelDevkitVersion();
	SceUID mod;
#ifdef ME_SOUND // [Shoey/Chilly]
	// have to do this before ME enabled or BOOOOOOOMMMMMM!!!!!!
	scePowerSetClockFrequency(333,333,166);


    sprintf(str,"%s%s",LaunchDir,"mediaengine.prx");
    if( (mod = pspSdkLoadStartModule(str, PSP_MEMORY_PARTITION_KERNEL)) < 0 )
    {
		ErrorExit(" Error  loading/mediaengine");
//        sceKernelDelayThread(3*1000*1000);
 		return 0;
    }

    me_data = (volatile struct me_struct*)malloc_64( sizeof( struct me_struct ) );  // [Shoey]
	if(!me_data)
	{
		ErrorExit(" malloc fail ME\n" );
		return 0;
	}
    me_data = (volatile struct me_struct*)(((int) me_data) | 0x40000000 );          // [Shoey]
    if( InitME( me_data, devkit_version ) )
    {
		ErrorExit(" Error Initializing ME\n" );
		return 0;
    }
#endif
#ifdef HOME_HOOK
//#ifndef ME_SOUND
//    // Don't call this if mediaengine.prx is already loaded
//    scePowerSetClockFrequency(333,333,166);
//#endif
    // Might want to set frequency here as well
    sprintf(str,"%s%s",LaunchDir,"homehook.prx");
	  if ( (mod = pspSdkLoadStartModule(str, PSP_MEMORY_PARTITION_KERNEL)) < 0)
	  {
		ErrorExit(" Error loading/homehook");
//        sceKernelDelayThread(3*1000*1000);
 		return 0;
	  }
    initHomeButton(devkit_version);
#endif

#ifndef FW3X
	//usb stuff
	//----------------
	loadUSBdrivers();
	//----------------
#endif

#ifdef ME_SOUND
//  me_data = me_struct_init();               // [jonny]
//  me_startproc((u32)me_function, (u32)me_data); // [jonny]
#endif
	// create user thread, tweek stack size here if necessary
#ifdef ME_SOUND
//me_stopproc();
#endif

	//user_main(0,NULL);
  //user thread for network
	SceUID g_mainthread = sceKernelCreateThread("User Mode Thread", user_main,
            0x9,
            256 * 1024, // stack size (256KB is regular default)
            PSP_THREAD_ATTR_USER|PSP_THREAD_ATTR_VFPU, NULL);

  // start user thread, then wait for it to do everything else
  sceKernelStartThread(g_mainthread, 0, NULL);
  sceKernelWaitThreadEnd(g_mainthread, NULL);

  sceKernelExitGame();
  return 0;
}



////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void low_level_init(){
	// init psp stuff
	pgFillAllvram(0);
	sceDisplaySetMode( 0, SCREEN_WIDTH, SCREEN_HEIGHT );
	sceDisplaySetFrameBuf( (char*)VRAM_ADDR, 512, 1, 1 );
	pgScreenFrame(2,0);
	pgFillAllvram(0);
	sceCtrlSetSamplingCycle( 0 );
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
#ifndef ME_SOUND
	scePowerSetClockFrequency(222,222,111);
#endif
#ifdef ME_SOUND
	scePowerLock(0); //sleep mode cannot be triggered with this :-)
#endif

#ifndef HOME_HOOK
	g_updatethread=SetupCallbacks();
#endif

	// create dirs if needed
	checkdirs();

	//init timezone, language, ...
	getsysparam();

	//blitter
	blit_init();


	os9x_adhoc_active=0;
	os9x_getnewfile=1;//start by choosing a file
	in_emu=0;
	os9x_notfirstlaunch=0;
	os9x_netplay=0;
	os9x_netpadindex=0;
	//default romPath is launch directory
	strcpy(romPath,LaunchDir);
	lastRom[0]=0;
	//do some uncaching stuff
	os9x_paused_ptr=(int*)UNCACHE_PTR(&os9x_paused);
	os9x_apuenabled_ptr=(int*)UNCACHE_PTR(&os9x_apuenabled);

	sceKernelDcacheWritebackInvalidateAll();
	(stSoundStatus.sound_fd) = -1;

	S9xAllocSound();
	S9xInitAPU();

	SetGeCallback();
	//OSK
	danzeff_load16(LaunchDir);

	//
	//sprintf(str_tmp,"%sDATA/msg.ini",LaunchDir);
	//save_msg_list(str_tmp);
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void low_level_deinit(){
	blit_shutdown();

	msgBoxLines(psp_msg_string(INFO_EXITING),0);

	for (int i=0;i<6;i++) {
		sceAudioChRelease(snd_beep1_handle[i]);
		//sceAudioChRelease(snd_beep2_handle[i]);
	}
	snd_beep1_current=0;
	//snd_beep2_current=0;

	S9xDeinitAPU();
	S9xFreeSound();

	//OSK
	danzeff_free();


	//network
	sceKernelDelayThread(1000000); //a try to fix hang on 2.5
}

int scroll_message_input(char *name,int limit) {
	/*int done = 0,i,j;
	// INIT OSK
	unsigned short intext[128]  = { 0 }; // text already in the edit box on start
	unsigned short outtext[128] = { 0 }; // text after input
	unsigned short desc[128]    = {'E','n','t','e','r',' ','S','t','r','i','n','g',' ','t','o',' ','f','i','n','d', 0 }; // description
	SceUtilityOskData data;
	SceUtilityOskParams osk;
	struct Vertex *vertices,*vertices_ptr;
	u16 *scr_bg=(u16*)(0x44000000+(512*272*2)*2);

	memset(&data, 0, sizeof(data));
	data.language = os9x_language; // english
	data.lines = 1; // just online
	data.unk_24 = 1; // set to 1
	data.desc = desc;
	data.intext = intext;
	data.outtextlength = 128; // sizeof(outtext) / sizeof(unsigned short)
	data.outtextlimit = limit; // just allow n chars
	data.outtext = outtext;

	memset(intext,0,128*2);
	for(i = 0; name[i]; i++) {
		intext[i]=name[i];
	}

	memset(&osk, 0, sizeof(osk));
	osk.size = sizeof(osk);
	osk.language = os9x_language;
	//if (os9x_language==LANGUAGE_JAPANESE)
	osk.buttonswap = 0;
	//else osk.buttonswap = 1;

	osk.unk_12 = 17; // What
	osk.unk_16 = 19; // the
	osk.unk_20 = 18; // fuck
	osk.unk_24 = 16; // ???
	osk.unk_48 = 1;
	osk.data = &data;

	// Only ascii code is handled so only the input of the small letters is printed

	int rc = sceUtilityOskInitStart(&osk);
	if(rc) {
		return 0;
	}
	while(!done) {

		sceGuStart(GU_DIRECT,list);
		sceGuEnable(GU_SCISSOR_TEST);
		sceGuEnable(GU_TEXTURE_2D);
		sceGuTexFilter(GU_NEAREST,GU_NEAREST);
		sceGuDisable(GU_DEPTH_TEST);
  	sceGuDisable(GU_ALPHA_TEST);
  	//sceGuDepthMask(GU_TRUE);
		sceGuTexScale(1.0f/512.0f,1.0f/512.0f);
		sceGuTexOffset(0,0);
		sceGuTexMode(GU_PSM_5551,0,0,0); //16bit texture
		sceGuScissor(0,0,480,272);
		sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);
		sceGuTexImage(0,512,512,512,(u8*)scr_bg);
  	vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
  	vertices_ptr=vertices;
		vertices_ptr[0].u = 0; vertices_ptr[0].v = 0;
		vertices_ptr[0].x = 0; vertices_ptr[0].y = 0; vertices_ptr[0].z = 0;
		vertices_ptr[1].u = 480; vertices_ptr[1].v = 272;
		vertices_ptr[1].x = 480; vertices_ptr[1].y = 272; vertices_ptr[1].z = 0;
		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,vertices);
		sceGuFinish();
  	sceGuSync(0,0);

		switch(sceUtilityOskGetStatus()) {
		case PSP_OSK_INIT :
			j=mh_length((unsigned char*)"Initializing OSK...");
			i=(480-j)/2;
			pgDrawFrame(i-5-1,125-1,i+j+5+1,145+1,12|(2<<5)|(2<<10));
  		pgDrawFrame(i-5-2,125-2,i+j+5+2,145+2,28|(10<<5)|(10<<10));
			pgFillBox(i-5,125,i+j+5,145,(20)|(4<<5)|(4<<10));
			mh_print(i,130,"Initializing OSK...",31|(28<<5)|(24<<10));
			break;
		case PSP_OSK_VISIBLE :
			sceUtilityOskUpdate(2); // 2 is taken from ps2dev.org recommendation
			break;
		case PSP_OSK_QUIT :
			sceUtilityOskShutdownStart();
			break;
		case PSP_OSK_FINISHED :
			done = 1;
			break;
		case PSP_OSK_NONE :
		default :
			break;
		}

		sceDisplayWaitVblankStart();
  	sceGuSwapBuffers();
		pg_drawframe++;
		pg_drawframe&=1;
	}

	if (data.rc==2) {				//new value input
		j=0;
		for(i = 0; data.outtext[i]; i++) {
			unsigned c = data.outtext[i];
			if(32 <= c && c <= 127) {
				//pspDebugScreenPrintf("%c", data.outtext[i]); // print ascii only
				name[j++]=c;
				if (j>=limit) break;
			}
		}
		name[j]=0;
	}
	return (data.rc==2);*/
	struct Vertex *vertices,*vertices_ptr;
	u16 *scr_bg=(u16*)(0x44000000+(512*272*2)*2);

	SceCtrlData paddata;
	//int oldmenufx;
	int exit_osk;
	unsigned char key,name_pos;
	//oldmenufx=os9x_menufx;
	//os9x_menufx=1;

//	danzeff_load16(LaunchDir);
	if (!danzeff_isinitialized()) {
		msgBoxLines("cannot init OSK",20);
		return 0;
	} else {
		danzeff_moveTo(20,20);
		exit_osk=0;
		name_pos=0;
		while (name[name_pos]) name_pos++;
		while (!exit_osk) {

			sceGuStart(GU_DIRECT,list);
		sceGuEnable(GU_SCISSOR_TEST);
		sceGuEnable(GU_TEXTURE_2D);
		sceGuTexFilter(GU_NEAREST,GU_NEAREST);
		sceGuDisable(GU_DEPTH_TEST);
  	sceGuDisable(GU_ALPHA_TEST);
  	//sceGuDepthMask(GU_TRUE);
		sceGuTexScale(1.0f/512.0f,1.0f/512.0f);
		sceGuTexOffset(0,0);
		sceGuTexMode(GU_PSM_5551,0,0,0); //16bit texture
		sceGuScissor(0,0,480,272);
		sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);
		sceGuTexImage(0,512,512,512,(u8*)scr_bg);
  	vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
  	vertices_ptr=vertices;
		vertices_ptr[0].u = 0; vertices_ptr[0].v = 0;
		vertices_ptr[0].x = 0; vertices_ptr[0].y = 0; vertices_ptr[0].z = 0;
		vertices_ptr[1].u = 480; vertices_ptr[1].v = 272;
		vertices_ptr[1].x = 480; vertices_ptr[1].y = 272; vertices_ptr[1].z = 0;
		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,vertices);
		sceGuFinish();
  	sceGuSync(0,0);

			sceCtrlPeekBufferPositive(&paddata, 1);
			switch (key=danzeff_readInput(paddata)) {
				case DANZEFF_START:exit_osk=1;break;
				case DANZEFF_SELECT:exit_osk=2;break;
				case 8://backspace
					if (name_pos>0) {
						name_pos--;
					}
					name[name_pos]=0;
					break;
				default:
					if (key>=32) {
						name[name_pos]=key;
						if (name_pos<limit-1) name_pos++;
							name[name_pos]=0;
					}
					break;
			}
			mh_printSel(200,20,(unsigned char*)name,0xFFFF);
			danzeff_render();

			sceDisplayWaitVblankStart();
  		sceGuSwapBuffers();
			pg_drawframe++;
			pg_drawframe&=1;
		}

		while (get_pad()) pgWaitV();

		return (exit_osk==1);
	}
	//os9x_menufx=oldmenufx;

//	danzeff_free();
}

int scroll_message(char **msg_lines,int lines,int start_pos,int intro_message,char *title) {
		int i,j,l,pos,end_pos,oldpos,fakedpos,col1,savedf;
		u16 *scr_bg,*src,*dst,*srctxt,*srctxt2;
		struct Vertex* vertices,*vertices_ptr;
		int found=0;
		char tofind[32];
		tofind[0]=0;
		//2 pages, draw frame is 0
		pgScreenFrame(2,1);

		//cheap scroller, using 3 screens

		srctxt=(u16*)(0x44000000+(512*272*3)*2);
		memset(srctxt,0x0,272*512*2*2);
		for (i=0;(i<lines)&&(i<27);i++) if (msg_lines[i]) {
			if (intro_message) {
				if (i<(lines-2)) col1=0xffff;
				else if (i==(lines-2)) col1=10|(31<<5)|(10<<10);
				else col1=31|(10<<5)|(10<<10);
			} else col1=31|(31<<5)|(31<<10);

			mh_printLimit(0,i*10+272*3,480,272*5,msg_lines[i],col1);
		}
		for (i=0;i<270*512*2;i++) {
			if (!(srctxt[i])) srctxt[i]=0xFFFF;
		}

		//now show messages & ask to scroll until the end
		scr_bg=(u16*)(0x04000000+(512*272*2)*2);//(u16*)malloc(480*272*2);
		dst=scr_bg;
		show_background(bg_img_mul,(os9x_lowbat?0x600000:0));

		pgFillBoxHalfer(0,0,479,9);
		pgDrawFrame(0,9,479,9,(12<<10)|(8<<5)|5);
		pgDrawFrame(0,10,479,10,(16<<10)|(14<<5)|14);
		pgDrawFrame(0,11,479,11,(12<<10)|(8<<5)|5);


		pgFillBoxHalfer(0,261,479,271);
		pgDrawFrame(0,259,479,259,(12<<10)|(8<<5)|5);
		pgDrawFrame(0,260,479,260,(16<<10)|(14<<5)|14);
		pgDrawFrame(0,261,479,261,(12<<10)|(8<<5)|5);


		mh_print(0,0,title,31|(31<<5)|(31<<10));
		sprintf(str_tmp,"   ,   to move -  ,  for fast mode");
		mh_print(479-mh_length((unsigned char*)str_tmp),0,(char*)str_tmp,31|(31<<5)|(31<<10));
		sprintf(str_tmp,"  " SJIS_UP "  " SJIS_DOWN "           L R              ");
		mh_print(479-mh_length((unsigned char*)str_tmp),0,(char*)str_tmp,20|(31<<5)|(18<<10));

		if (!intro_message) {
			sprintf(str_tmp,"   exit,        help");
			mh_print(479-mh_length((unsigned char*)str_tmp),262,(char*)str_tmp,31|(31<<5)|(31<<10));
			sprintf(str_tmp,SJIS_CROSS "       SELECT     ");
			mh_print(479-mh_length((unsigned char*)str_tmp),262,(char*)str_tmp,20|(31<<5)|(18<<10));
		}


		for (i=0;i<272;i++) {
			src = (u16*)pgGetVramAddr(0,i);
			memcpy(dst,src,480*2);
			dst+=512;
		}


		blit_reinit();
		oldpos=-10;
		pos=start_pos;

		int exit_message=0;
		int scroll_speed,scroll_accel;
		int pad_val,oldpad_val,lx,ly;
		scroll_accel=0;
		scroll_speed=1;
		pad_val=0;
		end_pos=(lines-26)*10;
		if (end_pos<0) end_pos=0;

		while (!exit_message) {

			fakedpos=pos%270;

			if (oldpos!=pos) {
				//scroll down / scroll down
				if (abs(oldpos-pos)>9) {
					pos=(pos/10)*10;
					fakedpos=pos%270;
					i=pos/10;
					j=fakedpos/10;
					srctxt=(u16*)(0x44000000+(512*(272*3+ j*10) )*2);
					memset(srctxt,0x0,272*512*2);

					savedf=pg_drawframe;
					pg_drawframe=0;
					for (l=i;(l<lines)&&(l<i+27);l++,j++) if (msg_lines[l]) {
						if (intro_message) {
							if (l<(lines-2)) col1=0xffff;
							else if (l==(lines-2)) col1=10|(31<<5)|(10<<10);
							else col1=31|(10<<5)|(10<<10);
						} else col1=31|(31<<5)|(31<<10);
						mh_printLimit(0,j*10+272*3,480,272*5,msg_lines[l],col1);
						//highlight searched string by drawing over a string with non searched part blanked
						if (found==l) {
							char *p,*q;
							strcpy(str_tmp,msg_lines[l]);
							p=str_tmp;
							while (q=strstr(strupr(p),tofind)) {
								while (p<q) *p++=' ';
								p=q+strlen(tofind);
							}
							while (*p) *p++=' ';
							col1=10|(10<<5)|(31<<10);
							mh_printLimit(0,j*10+272*3,480,272*5,str_tmp,col1);
						}
					}
					pg_drawframe=savedf;
					for (i=0;i<270*512;i++) {
						if (!(srctxt[i])) srctxt[i]=0xFFFF;
					}
					if (fakedpos) {
						srctxt=(u16*)(0x44000000+(512*(272*3+ fakedpos) )*2);
						srctxt2=(u16*)(0x44000000+(512*(272*3+ 270+fakedpos) )*2);
						memcpy(srctxt2,srctxt,(270-fakedpos)*512*2);

						srctxt=(u16*)(0x44000000+(512*(272*3+ 270) )*2);
						srctxt2=(u16*)(0x44000000+(512*(272*3+ 0) )*2);
						memcpy(srctxt2,srctxt,(270-fakedpos)*512*2);
					} else {
						srctxt=(u16*)(0x44000000+(512*(272*3)*2));
						srctxt2=(u16*)(0x44000000+(512*(272*3+ 270) )*2);
						memcpy(srctxt2,srctxt,270*512*2);
					}
				}
				i=pos/10+(oldpos>pos?0:27);
				j=fakedpos/10+(oldpos>pos?0:27);
				l=fakedpos/10+(oldpos>pos?27:0);
				if (i<lines) {
					if (intro_message) {
						if (i<(lines-2)) col1=0xffff;
						else if (i==(lines-2)) col1=10|(31<<5)|(10<<10);
						else col1=31|(10<<5)|(10<<10);
					} else col1=31|(31<<5)|(31<<10);

					srctxt=(u16*)(0x44000000+(512*(272*3+ j*10) )*2);
					srctxt2=(u16*)(0x44000000+(512*(272*3+l*10 ))*2);
					memset(srctxt,0x00,512*11*2);
					if (msg_lines[i]) {
						savedf=pg_drawframe;
						pg_drawframe=0;
						mh_printLimit(0,j*10+272*3,480,272*5,msg_lines[i],col1);
						pg_drawframe=savedf;
					}
					for (j=0;j<11*512;j++) {
						if (!(srctxt[j])) srctxt[j]=0xFFFF;
						//if (!(srctxt2[j])) srctxt2[j]=0xFFFF;
					}
					memcpy(srctxt2,srctxt,512*11*2);
				}
			}

			oldpos=pos;
			sceGuStart(GU_DIRECT,list);

			sceGuEnable(GU_SCISSOR_TEST);
			sceGuEnable(GU_TEXTURE_2D);
			sceGuTexFilter(GU_NEAREST,GU_NEAREST);
			sceGuDisable(GU_DEPTH_TEST);
  		sceGuDisable(GU_ALPHA_TEST);
  		//sceGuDepthMask(GU_TRUE);

			sceGuTexScale(1.0f/512.0f,1.0f/512.0f);
			sceGuTexOffset(0,0);
			sceGuTexMode(GU_PSM_5551,0,0,0); //16bit texture
			sceGuScissor(0,0,480,272);


			sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);
			sceGuTexImage(0,512,512,512,(u8*)scr_bg);

  		vertices = (struct Vertex*)sceGuGetMemory(2 * sizeof(struct Vertex));
  		vertices_ptr=vertices;
		  vertices_ptr[0].u = 0; vertices_ptr[0].v = 0;
		  vertices_ptr[0].x = 0; vertices_ptr[0].y = 0; vertices_ptr[0].z = 0;
		  vertices_ptr[1].u = 480; vertices_ptr[1].v = 272;
		  vertices_ptr[1].x = 480; vertices_ptr[1].y = 272; vertices_ptr[1].z = 0;
		  sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,vertices);

		  //sceGuTexFunc(GU_TFX_ADD,GU_TCC_RGBA);
		  sceGuEnable(GU_ALPHA_TEST);
		  sceGuAlphaFunc(GU_EQUAL,0,0x1);

		  sceGuScissor(0,12,480,259);
		  srctxt=(u16*)(0x44000000+(512*(272*3+fakedpos))*2);
		  sceGuTexImage(0,512,512,512,(u8*)srctxt);
		  sceGuDrawArray(GU_SPRITES,GU_TEXTURE_16BIT|GU_VERTEX_16BIT|GU_TRANSFORM_2D,2,0,vertices);

		  sceGuFinish();
  		sceGuSync(0,0);


  		//memset(pgGetVramAddr(0,272-10),0,512*10*2);
		  sprintf(str_tmp,"Line %d/%d  -  Page %d/%d",pos/10+26,lines,(pos/10+26)/27,lines/27);
		  mh_print(0,272-10,str_tmp,((31)|(28<<5)|(31<<10)));

  		sceDisplayWaitVblankStart();
  		sceGuSwapBuffers();
			pg_drawframe++;
			pg_drawframe&=1;

			for (;;) {
				oldpad_val=pad_val;
				while (!(pad_val=get_pad2(&lx,&ly))) {
					if (abs(lx-128)>20) break;
					if (abs(ly-128)>20) break;
					pgWaitV();scroll_speed=0;scroll_accel=0;
				}

				if ((pad_val&PSP_CTRL_RTRIGGER)&&(pos<end_pos)) {
					if (oldpad_val==pad_val) {
						if (scroll_accel<1024) {
							scroll_accel++;
							scroll_speed=(scroll_accel>>3)+1;
						}
					} else {
						scroll_speed=0;scroll_accel=0;
					}
					pos+=5*27*scroll_speed;
					if (pos>=end_pos) pos=end_pos;
					break;
				} else if ((pad_val&PSP_CTRL_LTRIGGER)&&(pos>0)) {
					if (oldpad_val==pad_val) {
						if (scroll_accel<1024) {
							scroll_accel++;
							scroll_speed=(scroll_accel>>3)+1;
						}
					} else {
						scroll_speed=0;scroll_accel=0;
					}
					pos-=5*27*scroll_speed;
					if (pos<0) pos=0;
					break;
				} else if ((pad_val&PSP_CTRL_DOWN)&&(pos<end_pos)) {
					if (oldpad_val==pad_val) {
						if (scroll_accel<256) {
							scroll_accel++;
							scroll_speed=(scroll_accel>>5)+1;
						}
					} else {
						scroll_speed=0;scroll_accel=0;
					}
					pos+=scroll_speed;
					if (pos>=end_pos) pos=end_pos;
					break;
				} else if ((pad_val&PSP_CTRL_UP)&&(pos>=0)) {
					if (oldpad_val==pad_val) {
						if (scroll_accel<256) {
							scroll_accel++;
							scroll_speed=(scroll_accel>>5)+1;
						}
					} else {
						scroll_speed=0;scroll_accel=0;
					}
					pos-=scroll_speed;
					if (pos<0) pos=0;
					break;
				} else if ((ly>=128+40)&&(pos<end_pos)) {
					int r=ly-128;
					if (r<127) pos+= r/16;
					else pos += 270;
					if (pos>end_pos) pos=end_pos;
					break;
				} else if ((ly<=128-20)&&(pos>0)) {
					int r=128-ly;
					if (r<127) pos-= r/16;
					else pos -= 270;
					if (pos<0) pos=0;
					break;
				}

				if (intro_message) {
					if (pos>=end_pos) { //reached the end of message
						if (pad_val&(PSP_CTRL_CIRCLE|PSP_CTRL_CROSS)) {exit_message=1;break;}
						if (pad_val&(PSP_CTRL_TRIANGLE|PSP_CTRL_SQUARE|PSP_CTRL_SQUARE/*|PSP_CTRL_LTRIGGER|PSP_CTRL_RTRIGGER*/)) {
							ExitCallback(0,0,NULL);
							exit_message=1;break;
						}
					}
				} else {
					if (pad_val&PSP_CTRL_CROSS) { //exit
						exit_message=1;
						break;
					}	else if (pad_val&PSP_CTRL_SELECT) { //minihelp
						msgBoxLines("Snes9xTYL - fileviewer\n\n" SJIS_TRIANGLE " Find, then " SJIS_CIRCLE " Find next, " SJIS_SQUARE " Find previous\n" \
						SJIS_UP "," SJIS_DOWN " scroll text, L,R scroll faster\n" SJIS_CROSS " exit\n\nLast position is keeped if same file is reopened.\nHowever it will be reset if another file is opened.\n\n" \
						"Press " SJIS_CROSS,0);
						while (!(get_pad()&PSP_CTRL_CROSS));
						while (get_pad());
						break;
					} else if (pad_val&PSP_CTRL_TRIANGLE) { //search from position
						if (scroll_message_input(tofind,31)) {
							found=0;
							if (tofind[0]) {
								msgBoxLines("Searching...",0);
								strcpy(tofind,strupr(tofind));
								j=pos/10-1;
								if (j<0) j=0;
								for (i=j;i<lines;i++)
									if (msg_lines[i]) {
										if (strstr(strupr(msg_lines[i]),tofind)) {found=i;pos=(i-2)*10;if (pos<0) pos=0;break;}
									}
							}
							if (!found) msgBoxLines("String not found!",30);
						}
						break;
					}	else if ((pad_val&PSP_CTRL_CIRCLE)&&found) { //search again from position & loop if needed
						msgBoxLines("Searching...",0);
						i=pos/10+2;
						if (i>=lines) i=0;
						j=i; //just to be safe, should not be needed
						for (;;) {
							i++;
							if (i>=lines) i=0;
							if (i==j) break; //just to be safe, should not be needed
							if (msg_lines[i]) {
								if (strstr(strupr(msg_lines[i]),tofind))  {found=i;pos=(i-2)*10;if (pos<0) pos=0;break;}
							}
						}
						break;
					}	else if ((pad_val&PSP_CTRL_SQUARE)&&found) { //search again from position & loop if needed
						msgBoxLines("Searching...",0);
						i=pos/10+2;
						if (i>=lines) i=0;
						j=i; //just to be safe, should not be needed
						for (;;) {
							i--;
							if (i<0) i=lines-1;
							if (i==j) break; //just to be safe, should not be needed
							if (msg_lines[i]) {
								if (strstr(strupr(msg_lines[i]),tofind))  {found=i;pos=(i-2)*10;if (pos<0) pos=0;break;}
							}
						}
						break;
					}
				}
			}
		}
		while (get_pad()) pgWaitV();
		return pos;
}
////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void show_message() {
#define BLANK_LINES 18
		char *decrypted_message,*p;
		int message_size;
		char password[32];
		char **msg_lines;
		int i,lines;


		strcpy(password,PASSWORD_XORED);
		message_size=sizeof(message);
		decrypted_message=(char*)malloc(message_size+1);
		decrypt((char*)message,decrypted_message,message_size,password);

		//pgFillBox(240-150+1,136-60+1,240+150-1,136+50-1,(16<<10)|(10<<5)|7);
		//pgDrawFrame(240-150,136-60,240+150,136+50,(12<<10)|(8<<5)|5);

		//prepare message to be diplayed

		//decrypt
		p=decrypted_message;
		p[message_size]=0;
		// 'lineify' it
		lines=0;
		while (*p) {
			i=0;
			//get new line
			while ((p[i]!=0x0D)&&(p[i])) i++;
			//if line carriage return, skip it & put a '0' / end of string
			if (p[i]==0x0D) {
				p[i]=0;p[i+1]=0; //0x0D 0x0A
				i+=2;
			}
			lines++;
			p=p+i;
		}

		lines+=BLANK_LINES;
		msg_lines=(char**)malloc(sizeof(char*)*lines);
		for (i=0;i<BLANK_LINES ;i++) {
			msg_lines[i]=NULL;
		}
		p=decrypted_message;
		for (i=BLANK_LINES ;i<lines;i++){
			msg_lines[i]=(char*)malloc(strlen(p)+1);
			strcpy(msg_lines[i],p);
			p=p+strlen(p)+2;
		}
		//free decrypted raw message
		free(decrypted_message);

		scroll_message(msg_lines,lines,0,1,"Disclaimer");

		//free 'linified' message
		for (i=0;i<lines;i++) if (msg_lines[i]) free(msg_lines[i]);
		free(msg_lines);

		//free(scr_bg);
}


////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void welcome_message(){
	char str[256];
	int res,show_msg=1;
	SceIoStat stat;
	ScePspDateTime *tfile;

	sprintf(str,"%s%s",LaunchDir,"s9xTYL.ini");
	res=sceIoGetstat("s9xTYL.ini",&stat);
	if (res<0) res=sceIoGetstat(str,&stat);
	if (res>=0) {

		tfile=&(stat.st_mtime);
		time_t cur_time;
		struct tm *tsys;
		sceKernelLibcTime(&cur_time);
		cur_time+=os9x_timezone*60+os9x_daylsavings*3600;;
		tsys=localtime(&cur_time);
		int diff=((abs(tsys->tm_mday-tfile->day)>=MESSAGE_DAYS)||(tsys->tm_mon+1!=tfile->month)||
						(tsys->tm_year+1900!=tfile->year));
		show_msg=0;
	}
	if (show_msg) show_message();

#ifdef ME_SOUND
	//if (show_msg) msgBoxLines("WARNING EXPERIMENTAL BETA VERSION\n\nSound is emulated by MEDIA ENGINE\n\nSLEEP MODE IS NOT SUPPORTED",2*60);
#endif

}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int os9x_getfile() {
#ifdef __DEBUG_SNES_
	sprintf(rom_filename,__DEBUG__ROM__);
	strcpy(LastPath,"ms0:/PSP/GAME/snes9xTYL/");
#else
	strcpy(rom_filename,lastRom);

	bypass_rom_settings=getFilePath(rom_filename,os9x_notfirstlaunch)-1;
	if (bypass_rom_settings<0) return 0;

	strcpy(lastRom,os9x_shortfilename(rom_filename));
	strcpy(romPath,LastPath);
#endif
	char *file_ext=strrchr((const char *)rom_filename,'/');
	if (!file_ext) file_ext=rom_filename;
  strcpy(shortrom_filename,file_ext+1);

	if (strcasecmp(os9x_filename_ext(rom_filename),"spc")==0) {
		return 2;
	}
	return 1;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int init_snes_rom() {
	///////////////////
	//Settings
	///////////////////
	memset( &Settings, 0, sizeof( Settings ) );
	memset( &Cheat, 0, sizeof( struct SCheatData ) );
	// ROM Options
	Settings.SDD1Pack = true;
  Settings.ForceLoROM = false;
  Settings.ForceInterleaved = false;
  Settings.ForceNotInterleaved = false;
  Settings.ForceInterleaved = false;
  Settings.ForceInterleaved2 = false;
  Settings.ForcePAL = false;
  Settings.ForceNTSC = false;
  Settings.ForceHeader = false;
  Settings.ForceNoHeader = false;
  // Sound options
  Settings.SoundSync = 0;
  Settings.InterpolatedSound = true;
  Settings.SoundEnvelopeHeightReading = true;
  Settings.DisableSoundEcho = false;
  Settings.DisableMasterVolume = false;
  Settings.Mute = FALSE;
  Settings.SoundSkipMethod = 0;
  Settings.SoundPlaybackRate = os9x_sndfreq;
  Settings.SixteenBitSound = true;
  Settings.Stereo = true;
  Settings.AltSampleDecode = 0;//os9x_sampledecoder;
  Settings.ReverseStereo = FALSE;
  Settings.SoundBufferSize = 1024;//4;
  Settings.SoundMixInterval = 0;//20;
	Settings.DisableSampleCaching=TRUE;
	Settings.FixFrequency = true;
	// Tracing options
  Settings.TraceDMA = false;
  Settings.TraceHDMA = false;
  Settings.TraceVRAM = false;
  Settings.TraceUnknownRegisters = false;
  Settings.TraceDSP = false;
  // Joystick options
  Settings.SwapJoypads = false;
  Settings.JoystickEnabled = false;
	// ROM timing options (see also H_Max above)
  Settings.PAL = false;
  Settings.FrameTimePAL = 20;
  Settings.FrameTimeNTSC = 17;
  // CPU options
  Settings.CyclesPercentage = os9x_CyclesPercentage;
  Settings.Shutdown = true;
  Settings.ShutdownMaster = true;
  Settings.NextAPUEnabled = Settings.APUEnabled = (os9x_apuenabled==1)||(os9x_apuenabled==2);
  Settings.DisableIRQ = os9x_DisableIRQ;
  Settings.Paused = false;
  Settings.H_Max = SNES_CYCLES_PER_SCANLINE;
  Settings.HBlankStart = (256 * Settings.H_Max) / SNES_HCOUNTER_MAX;
  if (os9x_fskipvalue==AUTO_FSKIP) {
  	Settings.SkipFrames=AUTO_FRAMERATE;
  	os9x_autofskip_SkipFrames=0;
  } else Settings.SkipFrames=os9x_fskipvalue;
  // ROM image and peripheral options
  Settings.ForceSuperFX = false;
  Settings.ForceNoSuperFX = false;
  Settings.MultiPlayer5 = true;
  Settings.Mouse = false;
  Settings.SuperScope = false;
  Settings.MultiPlayer5Master = false;
  Settings.SuperScopeMaster = false;
  Settings.MouseMaster = false;
  Settings.SuperFX = false;
  // SNES graphics options
  Settings.BGLayering = false;
  Settings.DisableGraphicWindows = false;
  Settings.ForceTransparency = false;
  Settings.ForceNoTransparency = false;
  Settings.DisableHDMA = os9x_DisableHDMA;
  Settings.Mode7Interpolate = true;
  Settings.DisplayFrameRate = false;

  Settings.SixteenBit = 1;
  Settings.Transparency = 1;
  Settings.SupportHiRes = false;

  Settings.AutoSaveDelay = 1;
  Settings.ApplyCheats = true;

  os9x_TurboMode = 0;
  Settings.TurboSkipFrames = 20;
  Settings.AutoMaxSkipFrames = 10;

  Settings.ForcedPause = 0;
  Settings.StopEmulation = TRUE;
  Settings.Paused = FALSE;
  Settings.HBlankStart = (256 * Settings.H_Max) / SNES_HCOUNTER_MAX;

  ///////////////////
  ///////////////////
  if (  !Memory.Init() ) {
  	msgBoxLines("Cannot init snes, memory issue",2*60);
		return -1;
	}
	S9xInitSound( Settings.SoundPlaybackRate, Settings.Stereo, Settings.SoundBufferSize );
	S9xSetSoundMute( TRUE );

	uint32 saved_flags = CPUPack.CPU.Flags;

	//msgBoxLines("Loading ROM...",0);
	
	//pgCopyScreen(); //R5 (Not needed?)

	if ( !Memory.LoadROM( rom_filename ) ){
		ErrorMsg("Error while loading rom");
	} else {
		Memory.LoadSRAM( (char*)S9xGetSaveFilename( ".SRM" ) );

		if (!bypass_rom_settings) {
			if (int ret=load_rom_settings(Memory.ROMCRC32)) {
				if (ret==-3) {
					msgBoxLines("!!Settings file not complete!!\n\nProbably coming from a previous version.\n\nNew settings will be set with default values",60*3);
				}
				else {
					msgBoxLines("No settings found, using default",10);
					if (load_rom_settings(0)) {
						if (!os9x_lowbat) save_rom_settings(0,"default");
					}
				}
			}
		} else {
			msgBoxLines("Forcing default settings",10);
			if (load_rom_settings(0)) {
					if (!os9x_lowbat) save_rom_settings(0,"default");
			}
		}


		//net stuff, called here to have settings loaded and so server can broadcast them
		//if (os9x_netplay) {
		//}

		if ((os9x_applyhacks)&&(os9x_findhacks(Memory.ROMCRC32))) {
			msgBox("Found speedhacks, applying...",30);
		}
	}

	CPUPack.CPU.Flags = saved_flags;

	S9xInitDisplay();
	if ( !S9xGraphicsInit() ){
		return -1;
	}




	if (os9x_apuenabled==2){
		S9xSetSoundMute( false );
	}
#ifdef __DEBUG_SNES_
	os9x_load(DEBUG_SAVE_SLOT);
#endif

	*os9x_paused_ptr=0;
	resync_var();
	InitSoundThread();
	if (os9x_apuenabled==2)	S9xSetSoundMute( false );
	else S9xSetSoundMute( true );

	in_emu=1;

	s_iFrame = 0;
	s_TotalFrame = 0;
	os9x_updatepadFrame = 0;
	os9x_snespad=0;
	os9x_oldsnespad=0;
	memset(os9x_netsnespad,0,sizeof(os9x_netsnespad));;memset(os9x_netcrc32,0,sizeof(os9x_netcrc32));
	os9x_netsynclost=0;
	os9x_oldframe=0;
	os9x_updatepadcpt=0;

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
void close_snes_rom(){
	*os9x_paused_ptr=1;
	StopSoundThread();
#ifndef ME_SOUND
	scePowerSetClockFrequency(222,222,111);
#endif
	Settings.Paused = TRUE;

	S9xGraphicsDeinit();
	Memory.Deinit();

	S9xCloseSoundDevice();
}


////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int user_main(SceSize args, void* argp) {
	low_level_init();

	initvar_withdefault();
	load_settings();

	/*special anim stuff*/
	//intro_anim();
	/**/

	load_background();

	load_icons();

	//welcome_message();

	filer_init("[" EMUNAME_VERSION "] - Choose a file",romPath);

	sprintf(os9x_viewfile_path,"%sFAQS/",LaunchDir);

	//sound stuff for menu
	for (int i=0;i<6;i++) {
		snd_beep1_handle[i]=sceAudioChReserve( -1, ((size_snd_beep2-44)/8)&(~63), 0 );
		//snd_beep2_handle[i]=sceAudioChReserve( -1, ((size_snd_beep2-44)/4)&(~63), 0 );
	}

	while ( g_bLoop ) {

		if (os9x_getnewfile) {
			pgFillAllvram(0);
			pgScreenFrame(2,0);

			if (in_emu==1) {//currently emulating a game => save SRAM before loading new one
				if (!os9x_lowbat) {
					Memory.SaveSRAM( (char*)S9xGetSaveFilename(".SRM") );
					//S9xSaveCheatFile( (char*)S9xGetSaveFilename( ".cht" ) );
				}
				before_pause();
			}

			//initUSBdrivers();

			//1 is snes rom, 2 is spc file
			switch (os9x_getfile()) {
				case 0:
					if (in_emu==1) {
						after_pause();
					}
					os9x_getnewfile=0;
					pgFillAllvram(0);pgScreenFrame(2,0);
					break;
				case 1:
					if (in_emu==1) { //a snes rom was being emulated, close stuff
						close_snes_rom();
						///hmm here? ...
						in_emu=0;
					}
					if (init_snes_rom()) {
						msgBoxLines("Cannot initialize ROM",60*2);
						close_snes_rom();
					} else {
						os9x_getnewfile=0;
						os9x_notfirstlaunch=1;
						pgFillAllvram(0);pgScreenFrame(2,0);
					}
				break;
				case 2:
					if (in_emu==1) { //a snes rom was being emulated, close stuff
						close_snes_rom();
						in_emu=0;
					}
					pgFillAllvram(0);pgScreenFrame(2,0);

					in_emu=2;
					//play spc, blocking
					msgBoxLines("Playing spc file...",0);
					OSPC_Play(rom_filename,0,MAXVOLUME);
					blit_reinit();
					set_cpu_clock();

					//ask for a new file since we finished playing the current one
					os9x_getnewfile=1;
					os9x_notfirstlaunch=0;
					pgFillAllvram(0);pgScreenFrame(2,0);
					in_emu=0;
				break;
			}

#ifndef FW3X
			endUSBdrivers();
#endif
		}

		if ((in_emu==1) && ( !Settings.Paused )){
			S9xMainLoop();
		}
		if (g_bSleep){
#ifdef ME_SOUND
			os9x_specialaction|=OS9X_MENUACCESS;
			S9xProcessEvents(false);
#else
			while(g_bSleep) pgWaitV();
			pgWaitVn(60*3);//give some times to wake up, 3seconds
			resync_var();
			InitSoundThread();
			if (os9x_apuenabled==2)	S9xSetSoundMute( false );
			else S9xSetSoundMute( true );
			*os9x_paused_ptr=0;
#endif
		}
#ifdef HOME_HOOK
    if( readHomeButton() > 0 )
    {
			os9x_specialaction|=OS9X_MENUACCESS;
			S9xProcessEvents(false);
    }
#endif
	}

	if (!os9x_lowbat) {
		save_settings();
		if (in_emu==1) {
			Memory.SaveSRAM( (char*)S9xGetSaveFilename(".SRM") );
			//S9xSaveCheatFile( (char*)S9xGetSaveFilename( ".cht" ) );
			save_rom_settings(Memory.ROMCRC32,Memory.ROMName);
			close_snes_rom();
			in_emu=0;
		}
	}

	low_level_deinit();

	if (bg_img) image_free(bg_img);
	for (int i=0;i<7;i++) if (icons[i]) {free(icons[i]);icons[i]=NULL;}

	return 0;
}


}
/////////////////////////////////////////////////////////////////////
// ?J?E???^????

uint32 g_nCount;
clock_t g_ulStart;

void MyCounter_Init(void)
{
	g_nCount = 0;
	g_ulStart = sceKernelLibcClock();
}
void MyCounter_drawCount()
{
	//if(!os9x_showcounter)
	//	return;
	if (g_ulStart != 0xFFFFFFFF) {
		clock_t dwTime = sceKernelLibcClock();
		if (20000000 < dwTime - g_ulStart) {
			g_ulStart = 0xFFFFFFFF;
		}
		else {
			g_nCount++;
		}
	}
	char szBuf[16];
	sprintf(szBuf,"%d",g_nCount);
	pgPrintBG(0,0,0xffff,szBuf);
}

