#include "psp.h"

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int os9x_savesnap() {
	u16 *snes_image;
	if (os9x_softrendering<2)	snes_image=(u16*)(0x44000000+512*272*2*2);
	else snes_image=(u16*)(0x44000000+2*512*272*2+256*240*2+2*256*256*2);
		
	write_JPEG_file ((char*)S9xGetSaveFilename(".jpg"),75,snes_image,256,os9x_snesheight,256); 
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int os9x_loadsnap(char *fname,u16 *snes_image,int *height) {
	if (read_JPEG_file (fname,snes_image,256,240,256,height)) return 1;	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int os9x_savesram() {
	Memory.SaveSRAM( (char*)S9xGetSaveFilename(".SRM") );
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int os9x_save(const char *ext)
{
	const char *save_filename;
	
	os9x_externstate_mode=0;
	save_filename=S9xGetSaveFilename (ext);	
	//msgBoxLines((char*)save_filename,10);	
	S9xFreezeGame(save_filename);	
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int os9x_load(const char *ext) {	
	FILE *savefile;
	const char *save_filename;
	os9x_externstate_mode=0;
	
	save_filename=S9xGetSaveFilename (ext);	
	savefile=fopen(save_filename,"rb");
  if (savefile) {
  	fclose(savefile);
		S9xUnfreezeGame(save_filename);
//		S9xInitUpdate();
		S9xReschedule ();
		return 1;
	} 
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int os9x_loadfname(const char *fname) {	
	FILE *savefile;
	char *ext;
	ext=strrchr(fname,'.');
	if (strlen(ext)==4) {				
		if (!strcasecmp(ext,".srm")) {
			msgBoxLines("Found an SRAM file",60);
			Memory.LoadSRAM( (char*)fname );	
			return 1;
		}
	}
	
	os9x_externstate_mode=1;
	savefile=fopen(fname,"rb");
  if (savefile) {
  	fclose(savefile);
		S9xUnfreezeGame(fname);
//		S9xInitUpdate();
		S9xReschedule ();
		return 1;
	} 
	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////////////
int os9x_loadzsnes(char *ext) {	
	FILE *savefile;
	const char *save_filename;
	
	os9x_externstate_mode=1;
	save_filename=S9xGetSaveFilename (ext);	
	savefile=fopen(save_filename,"rb");
  if (savefile) {
 		fclose(savefile);		
		S9xUnfreezeGame(save_filename);
//		S9xInitUpdate();
		S9xReschedule ();
		return 1;
	}
	return 0;
}
