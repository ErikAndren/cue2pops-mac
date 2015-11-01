/* 	BIN/CUE to IMAGE0.VCD conversion tool v2.0
	Last modified : 2013/05/16
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <inttypes.h>
#include <time.h>

#ifdef DEBUG
  const int debug = 1;
#else
  const int debug = 0;
#endif

const int batch = 0;			// Else than zero, user prompt is disabled and CDRWIN image fix is ENABLED. Doesn't halt on anything. Suitable for batch execution.

FILE *file, *bin_file; //file is used for opening the input cue and the output file, bin_file is used for opening the BIN that's attached to the cue.
char *bin_path; // name/path of the BIN that is attached to the cue. Handled by the parser then altered if it doesn't contain the full path.
const int sectorsize = 2352; // Sector size

int vmode = 0; // User command status (vmode)
int trainer = 0; // User command status (trainer)
int fix_CDRWIN = 0; // Special CDRWIN pregap injection status
int bin_size; // BIN (disc image) size
int sector_count; // Calculated number of sectors
int track_count = 0;; // Number of "TRACK " occurrences in the cue
int pregap_count = 0;; // Number of "PREGAP" occurrences in the cue
int postgap_count = 0; // Number of "POSTGAP" occurrences in the cue
int daTrack_ptr = 0; // Indicates the location of the pregap that's between the data track and the first audio track
int headersize = 0x100000; // POPS header size. Also used as buffer size for caching BIN data in file output operations

int deny_vmode = 0; 	// 2013/05/16 - v2.0 : Triggered by GameIdentifier... Makes NTSCpatcher skip the PAL->NTSC patch.
int fix_game = 0;		// 2013/05/16 - v2.0 : Triggered by GameIdentifier... Enables GameFixer .
int GameHasCheats = 0;	// 2013/05/16 - v2.0 : Triggered by GameIdentifier... .

int GameTitle = 0;
int GameTrained = 0;
int GameFixed = 0;



void GameIdentifier(unsigned char *inbuf)
{
	int ptr;

	if(debug != 0) {
		if(vmode == 0) printf("----------------------------------------------------------------------------------\n");
		printf("Hello from GameIdentifier !\n");
	}

	if(GameTitle == 0) {
		for(ptr = 0; ptr < headersize; ptr += 4) {
			if(inbuf[ptr] == 'S' && inbuf[ptr+1] == 'C' && inbuf[ptr+2] == 'E' && inbuf[ptr+3] == 'S' && inbuf[ptr+4] == '-' && inbuf[ptr+5] == '0' && inbuf[ptr+6] == '0' && inbuf[ptr+7] == '3' && inbuf[ptr+8] == '4' && inbuf[ptr+9] == '4' && inbuf[ptr+10] == ' ' && inbuf[ptr+11] == ' ' && inbuf[ptr+12] == ' ' && inbuf[ptr+13] == ' ' && inbuf[ptr+14] == ' ' && inbuf[ptr+15] == ' ') {
				if(debug == 0) printf("----------------------------------------------------------------------------------\n");
				printf("Crash Bandicoot [SCES-00344]\n");
				deny_vmode ++; // 2013/05/16 - v2.0 : The NTSC patch fucks up the framerate badly, so now it's skipped
				GameTitle = 1;
				GameHasCheats = 1;
				fix_game = 0;
				break;
			}
			/* -------------------------------- */
			if(inbuf[ptr] == 'S' && inbuf[ptr+1] == 'C' && inbuf[ptr+2] == 'U' && inbuf[ptr+3] == 'S' && inbuf[ptr+4] == '-' && inbuf[ptr+5] == '9' && inbuf[ptr+6] == '4' && inbuf[ptr+7] == '9' && inbuf[ptr+8] == '0' && inbuf[ptr+9] == '0' && inbuf[ptr+10] == ' ' && inbuf[ptr+11] == ' '&& inbuf[ptr+12] == ' ' && inbuf[ptr+13] == ' ' && inbuf[ptr+14] == ' ' && inbuf[ptr+15] == ' ') {
				if(debug == 0) printf("----------------------------------------------------------------------------------\n");
				printf("Crash Bandicoot [SCUS-94900]\n");
				GameTitle = 2;
				GameHasCheats = 1;
				fix_game = 0;
				break;
			}
			/* -------------------------------- */
			if(inbuf[ptr] == 'S' && inbuf[ptr+1] == 'C' && inbuf[ptr+2] == 'P' && inbuf[ptr+3] == 'S' && inbuf[ptr+4] == '_' && inbuf[ptr+5] == '1' && inbuf[ptr+6] == '0' && inbuf[ptr+7] == '0' && inbuf[ptr+8] == '3' && inbuf[ptr+9] == '1' && inbuf[ptr+10] == ' ' && inbuf[ptr+11] == ' ' && inbuf[ptr+12] == ' ' && inbuf[ptr+13] == ' ' && inbuf[ptr+14] == ' ' && inbuf[ptr+15] == ' ') {
				if(debug == 0) printf("----------------------------------------------------------------------------------\n");
				printf("Crash Bandicoot [SCPS-10031]\n");
				GameTitle = 3;
				GameHasCheats = 1;
				fix_game = 0;
				break;
			}
			/* -------------------------------- */
			if(inbuf[ptr] == ' ' && inbuf[ptr+1] == '1' && inbuf[ptr+2] == '9' && inbuf[ptr+3] == '9' && inbuf[ptr+4] == '9' && inbuf[ptr+5] == '0' && inbuf[ptr+6] == '8' && inbuf[ptr+7] == '1' && inbuf[ptr+8] == '6' && inbuf[ptr+9] == '1' && inbuf[ptr+10] == '4' && inbuf[ptr+11] == '1' && inbuf[ptr+12] == '6' && inbuf[ptr+13] == '3' && inbuf[ptr+14] == '3' && inbuf[ptr+15] == '0' && inbuf[ptr+16] == '0' && inbuf[ptr+17] == '$') {
				if(debug == 0) printf("----------------------------------------------------------------------------------\n");
				printf("Metal Gear Solid : Special Missions [SLES-02136]\n");
				GameTitle = 4;
				GameHasCheats = 0;
				fix_game = 1;
				break;
			}
		}
	}

	if(GameTitle != 0 && fix_game == 1) printf("GameFixer is ON\n");
	if(GameTitle != 0 && trainer == 1 && GameHasCheats == 0) printf("There is no cheat for this title\n");
	if(GameTitle != 0 && deny_vmode != 0 && vmode == 1) printf("VMODE patching is disabled for this title\n");
	if(GameTitle != 0 && debug == 0) printf("----------------------------------------------------------------------------------\n");

	if(debug != 0) {
		printf("GameTitle     = %d\n", GameTitle);
		printf("fix_game      = %d\n", fix_game);
		printf("deny_vmode    = %d\n", deny_vmode);
		printf("GameHasCheats = %d\n", GameHasCheats);
		printf("GameIdentifier says goodbye.\n");
		printf("----------------------------------------------------------------------------------\n");
	}

	if(GameTitle == 0 && trainer == 1) {
		printf("Unknown game, no fix/trainer enabled\n");
		printf("Continuing...\n");
	}
}


void GameFixer(unsigned char *inbuf)
{
	int ptr;

	if(GameFixed == 0) {
		for(ptr = 0; ptr < headersize; ptr += 4) {
			if(GameTitle == 4) {
				if(inbuf[ptr] == 0x78 && inbuf[ptr+1] == 0x26 && inbuf[ptr+2] == 0x43 && inbuf[ptr+3] == 0x8C) inbuf[ptr] = 0x74;
				if(inbuf[ptr] == 0xE8 && inbuf[ptr+1] == 0x75 && inbuf[ptr+2] == 0x06 && inbuf[ptr+3] == 0x80) {
					inbuf[ptr-8] = 0x07;
					printf("GameFixer : Disc Swap Patched\n");
					GameFixed = 1;
					printf("----------------------------------------------------------------------------------\n");
					break;
				}
			}
		}
	}
}


void GameTrainer(unsigned char *inbuf)
{
	int ptr;

	if(GameTrained == 0) {
		for(ptr = 0; ptr < headersize; ptr += 4) {
			if(GameTitle == 1) {
				if(inbuf[ptr] == 0x7C && inbuf[ptr+1] == 0x16 && inbuf[ptr+2] == 0x20 && inbuf[ptr+3] == 0xAC) {
					inbuf[ptr+2] = 0x22;
					printf("GameTrainer : Test Save System Enabled\n");
					GameTrained = 1;
					printf("----------------------------------------------------------------------------------\n");
					break;
				}
			}
			if(GameTitle == 2) {
				if(inbuf[ptr] == 0x9C && inbuf[ptr+1] == 0x19 && inbuf[ptr+2] == 0x20 && inbuf[ptr+3] == 0xAC) {
					inbuf[ptr+2] = 0x22;
					printf("GameTrainer : Test Save System Enabled\n");
					GameTrained = 1;
					printf("----------------------------------------------------------------------------------\n");
					break;
				}
			}
			if(GameTitle == 3) {
				if(inbuf[ptr] == 0x84 && inbuf[ptr+1] == 0x19 && inbuf[ptr+2] == 0x20 && inbuf[ptr+3] == 0xAC) {
					inbuf[ptr+2] = 0x22;
					printf("GameTrainer : Test Save System Enabled\n");
					GameTrained = 1;
					printf("----------------------------------------------------------------------------------\n");
					break;
				}
			}
		}
	}
}


void NTSCpatcher(unsigned char *inbuf, int tracker)
{
	int ptr;

	for(ptr = 0; ptr < headersize; ptr += 4) {
		if((inbuf[ptr] == 0x13 && inbuf[ptr+1] == 0x00 && (inbuf[ptr+2] == 0x90 || inbuf[ptr+2] == 0x91) && inbuf[ptr+3] == 0x24) && (inbuf[ptr+4] == 0x10 && inbuf[ptr+5] == 0x00 && (inbuf[ptr+6] == 0x90 || inbuf[ptr+6] == 0x91) && inbuf[ptr+7] == 0x24)) {
			// ?? 00 90 24 ?? 00 90 24 || ?? 00 91 24 ?? 00 91 24 || ?? 00 91 24 ?? 00 90 24 || ?? 00 90 24 ?? 00 91 24
			printf("Y-Pos pattern found at dump offset 0x%X / LBA %d (VCD offset 0x%X)\n", tracker + ptr, (tracker + ptr) / sectorsize, headersize + tracker + ptr);
			inbuf[ptr] = 0xF8;		// 2013/05/16, v2.0 : Also apply the fix here, in case NTSCpatcher cannot find/patch the video mode
			inbuf[ptr+1] = 0xFF;	// 2013/05/16, v2.0 : //
			inbuf[ptr+4] = 0xF8;
			inbuf[ptr+5] = 0xFF;
			printf("----------------------------------------------------------------------------------\n");
		} else if(inbuf[ptr+2] != 0xBD && inbuf[ptr+3] != 0x27 && inbuf[ptr+4] == 0x08 && inbuf[ptr+5] == 0x00 && inbuf[ptr+6] == 0xE0 && inbuf[ptr+7] == 0x03 && inbuf[ptr+14] == 0x02 && inbuf[ptr+15] == 0x3C && inbuf[ptr+18] == 0x42 && inbuf[ptr+19] == 0x8C && inbuf[ptr+20] == 0x08 && inbuf[ptr+21] == 0x00 && inbuf[ptr+22] == 0xE0 && inbuf[ptr+23] == 0x03 && inbuf[ptr+24] == 0x00 && inbuf[ptr+25] == 0x00 && inbuf[ptr+26] == 0x00 && inbuf[ptr+27] == 0x00 && ((inbuf[ptr+2] == 0x24 && inbuf[ptr+3] == 0xAC) || (inbuf[ptr+6] == 0x24 && inbuf[ptr+7] == 0xAC) || (inbuf[ptr+10] == 0x24 && inbuf[ptr+11] == 0xAC))) {
			// ?? ?? ?? ?? 08 00 E0 03 ?? ?? ?? ?? ?? ?? 02 3C ?? ?? 42 8C 08 00 E0 03 00 00 00 00
			if(deny_vmode != 0) printf("Skipped VMODE pattern at dump offset 0x%X / LBA %d (VCD offset 0x%X)\n", tracker + ptr, (tracker + ptr) / sectorsize, headersize + tracker + ptr);
			if(deny_vmode == 0) {
				printf("VMODE pattern found at dump offset 0x%X / LBA %d (VCD offset 0x%X)\n", tracker + ptr, (tracker + ptr) / sectorsize, headersize + tracker + ptr);
				inbuf[ptr+12] = 0x00;
				inbuf[ptr+13] = 0x00;
				inbuf[ptr+14] = 0x00;
				inbuf[ptr+15] = 0x00;
				inbuf[ptr+16] = 0x00;
				inbuf[ptr+17] = 0x00;
				inbuf[ptr+18] = 0x02;
				inbuf[ptr+19] = 0x24;
				if(inbuf[ptr+2] == 0x24 && inbuf[ptr+3] == 0xAC) inbuf[ptr+2] = 0x20;
				else if(inbuf[ptr+6] == 0x24 && inbuf[ptr+7] == 0xAC) inbuf[ptr+6] = 0x20;
				else if(inbuf[ptr+10] == 0x24 && inbuf[ptr+11] == 0xAC) inbuf[ptr+10] = 0x20;
			}
			printf("----------------------------------------------------------------------------------\n");
		}
	}
}


int GetLeadOut(unsigned char *hbuf)
{
	/* MSF is calculated from the dump size so DO NOT APPLY gap++/gap-- ADJUSTMENTS IN THIS FUNCTION ! */
	FILE *bin;
	int status;
	
	// Formatted Lead-Out MM:SS:FF
	char LeadOut[7];
	int leadoutM; // Calculated Lead-Out MM:__:__
	int leadoutS; // Calculated Lead-Out __:SS:__
	int leadoutF; // Calculated Lead-Out __:__:FF
	
	if(!(bin = fopen(bin_path, "rb"))) { // Open the BINARY that is attached to the cue
		printf("Error: Cannot open %s\n\n", bin_path);
		return -1;
	}
	
	status = fseek(bin, 0, SEEK_END);
	if (status != 0) {
		printf("Error: Failed to seek %s\n", bin_path);
		return -1;
	}

	bin_size = ftell(bin); // Get it's size
	if (bin_size == -1L) {
		printf("Error: Failed to get file %s size\n", bin_path);
		return -1;
	}
	fclose(bin);

	sector_count = (bin_size / sectorsize) + (150 * (pregap_count + postgap_count)) + 150; // Convert the bin_size to sector count
	leadoutM = sector_count / 4500;
	leadoutS = (sector_count - leadoutM * 4500) / 75;
	leadoutF = sector_count - leadoutM * 4500 - leadoutS * 75;
	if(debug != 0) {
		printf("Calculated Lead-Out MSF = %02d:%02d:%02d\n", leadoutM, leadoutS, leadoutF);
	}
	sector_count = (bin_size / sectorsize) + (150 * (pregap_count + postgap_count));
	if(debug != 0) {
		printf("Calculated Sector Count = %08Xh (%i)\n", sector_count, sector_count);
	}
	
	// Additonally we can add a dbg printf of the sector count that's written in sector 16 for verification. Mmmm kinda waste of time
	/* Tired of math already. sprintf + redo what was done with the cue sheet MSFs */
	sprintf(&LeadOut[0], "%02d", leadoutM);
	sprintf(&LeadOut[2], "%02d", leadoutS);
	sprintf(&LeadOut[4], "%02d", leadoutF);
	
	hbuf[27] = ((LeadOut[0] - 48) * 16) + (LeadOut[1] - 48);
	hbuf[28] = ((LeadOut[2] - 48) * 16) + (LeadOut[3] - 48);
	hbuf[29] = ((LeadOut[4] - 48) * 16) + (LeadOut[5] - 48);
	if(debug != 0) {
		printf("Formatted Lead-Out MSF  = %02X:%02X:%02X\n\n", hbuf[27], hbuf[28], hbuf[29]);
	}

	return 0;
}


int main(int argc, char **argv)
{
	char *cuebuf; // Buffer for the cue sheet
	int cuesize; // Size of the cue sheet
	int cue_ptr;  // Indicates the location of the current INDEX 01 entry in the cue sheet
	int binary_count = 0; // Number of "BINARY" occurrences in the cue
	int index0_count = 0; // Number of "INDEX 00" occurrences in the cue
	int index1_count = 0; // Number of "INDEX 01" occurrences in the cue
	int wave_count = 0; // Number of "WAVE" occurrences in the cue
	char *ptr; // Pointer to the Track 01 type in the cue. Used to set the sector size, the disc type or to reject the cue
	char answer[3]; // Where the user answer is stored. Used in the CDRWIN fix prompt shit

	unsigned char *headerbuf; // Buffer for the POPS header
	unsigned char outbuf[headersize]; // File I/O cache
	int header_ptr = 20; // Integer that points to a location of the POPS header buffer. Decimal 20 is the start of the descriptor "A2"
	int i; // Tracker
	int m; // Calculated and formatted MM:__:__ of the current index
	int s; // Calculated and formatted __:SS:__ of the current index
	int f; // Calculated and formatted __:__:FF of the current index
	int noCDDA = 0; // 2013/04/22 - v1.2 : Is set to 1 if no CDDA track was found in the game dump, used by the NTSC patcher

	int gap_ptr = 0; // Indicates the location of the current INDEX 00 entry in the cue sheet
	int gap_more = 0; // User command status (gap++)
	int gap_less = 0; // User command status (gap--)

	printf("\nBIN/CUE to IMAGE0.VCD conversion tool v2.0\n");
	printf("Last modified: %s\n\n", __DATE__);

	if(argc <= 1) {
		printf("Error: No input file specified (cue sheet)\n\n");
		printf("Usage :\n");
		printf("%s input.cue <cmd_1> <cmd_2> <cmd_3> <output.vcd>\n\n", argv[0]);
		printf("Commands are :\n");
		printf("gap++ : Adds 2 seconds to all track indexes MSF\n");
		printf("gap-- : Substracts 2 seconds to all track indexes MSF\n");
		printf("vmode : Attempts to patch the video mode to NTSC and to fix the screen position\n");
		printf("trainer : Enable cheats\n\n");
		printf("Examples :\n");
		printf("%s mygame.cue\n", argv[0]);
		printf("%s mygame.cue IMAGE0.VCD\n", argv[0]);
		printf("%s mygame.cue vmode\n", argv[0]);
		printf("%s mygame.cue gap++\n", argv[0]);
		printf("%s mygame.cue gap--\n", argv[0]);
		printf("%s mygame.cue trainer\n", argv[0]);
		printf("%s mygame.cue vmode IMAGE0.VCD\n", argv[0]);
		printf("%s mygame.cue gap++ IMAGE0.VCD\n", argv[0]);
		printf("%s mygame.cue gap-- IMAGE0.VCD\n", argv[0]);
		printf("%s mygame.cue trainer IMAGE0.VCD\n", argv[0]);
		printf("%s mygame.cue gap++ vmode\n", argv[0]);
		printf("%s mygame.cue gap++ trainer\n", argv[0]);
		printf("%s mygame.cue gap-- vmode\n", argv[0]);
		printf("%s mygame.cue gap-- trainer\n", argv[0]);
		printf("%s mygame.cue gap++ vmode IMAGE0.VCD\n", argv[0]);
		printf("%s mygame.cue gap-- vmode IMAGE0.VCD\n", argv[0]);
		printf("%s mygame.cue gap++ trainer IMAGE0.VCD\n", argv[0]);
		printf("%s mygame.cue gap-- trainer IMAGE0.VCD\n", argv[0]);
		printf("%s mygame.cue gap++ vmode trainer IMAGE0.VCD\n", argv[0]);
		printf("%s mygame.cue gap-- vmode trainer IMAGE0.VCD\n", argv[0]);
		printf("Commands and output file are optional.\n\n");
		return 0;
	}

	// if(argc == 2), PROGRAM.EXE INPUT.CUE and make the output file name from the input file name

	if(argc == 3) { // PROGRAM.EXE INPUT.CUE <CMD_1/OUTPUT>
		if(!strcmp(argv[2], "gap++")) 	gap_more = 1;
		if(!strcmp(argv[2], "gap--")) 	gap_less = 1;
		if(!strcmp(argv[2], "vmode")) 	vmode = 1;
		if(!strcmp(argv[2], "trainer")) trainer = 1;
		// else, argv[2] is the output file name
	}

	if(argc == 4) { // PROGRAM.EXE INPUT.CUE <CMD_1> <CMD_2/OUTPUT>
		if(!strcmp(argv[2], "gap++")) 			gap_more = 1;
		else if(!strcmp(argv[2], "gap--")) 		gap_less = 1;
		else if(!strcmp(argv[2], "vmode")) 		vmode = 1;
		else if(!strcmp(argv[2], "trainer")) 	trainer = 1;
		else {
			printf("Syntax Error : Argument %d (%s) is not valid.\n\n", argc - 3 , argv[2]);
			return 0;
		}
		if(!strcmp(argv[3], "gap++")) 			gap_more = 1;
		else if(!strcmp(argv[3], "gap--")) 		gap_less = 1;
		else if(!strcmp(argv[3], "vmode")) 		vmode = 1;
		else if(!strcmp(argv[3], "trainer")) 	trainer = 1;
		// else, argv[3] is the output file name
	}

	if(argc == 5) { // PROGRAM.EXE INPUT.CUE <CMD_1> <CMD_2> <CMD_3/OUTPUT>
		if(!strcmp(argv[2], "gap++")) 			gap_more = 1;
		else if(!strcmp(argv[2], "gap--")) 		gap_less = 1;
		else if(!strcmp(argv[2], "vmode")) 		vmode = 1;
		else if(!strcmp(argv[2], "trainer")) 	trainer = 1;
		else {
			printf("Syntax Error : Argument %d (%s) is not valid.\n\n", argc - 4 , argv[2]);
			return 0;
		}
		if(!strcmp(argv[3], "gap++")) 			gap_more = 1;
		else if(!strcmp(argv[3], "gap--")) 		gap_less = 1;
		else if(!strcmp(argv[3], "vmode")) 		vmode = 1;
		else if(!strcmp(argv[3], "trainer")) 	trainer = 1;
		else {
			printf("Syntax Error : Argument %d (%s) is not valid.\n\n", argc - 3 , argv[3]);
			return 0;
		}
		if(!strcmp(argv[4], "gap++")) 			gap_more = 1;
		else if(!strcmp(argv[4], "gap--"))	 	gap_less = 1;
		else if(!strcmp(argv[4], "vmode")) 		vmode = 1;
		else if(!strcmp(argv[4], "trainer")) 	trainer = 1;
		// else, argv[4] is the output file name
	}

	if(argc == 6) { // PROGRAM.EXE INPUT.CUE <CMD_1> <CMD_2> <CMD_3> <OUTPUT>
		if(!strcmp(argv[2], "gap++")) 			gap_more = 1;
		else if(!strcmp(argv[2], "gap--")) 		gap_less = 1;
		else if(!strcmp(argv[2], "vmode")) 		vmode = 1;
		else if(!strcmp(argv[2], "trainer")) 	trainer = 1;
		else {
			printf("Syntax Error : Argument %d (%s) is not valid.\n\n", argc - 4 , argv[2]);
			return 0;
		}
		if(!strcmp(argv[3], "gap++")) 			gap_more = 1;
		else if(!strcmp(argv[3], "gap--"))	 	gap_less = 1;
		else if(!strcmp(argv[3], "vmode")) 		vmode = 1;
		else if(!strcmp(argv[3], "trainer")) 	trainer = 1;
		else {
			printf("Syntax Error : Argument %d (%s) is not valid.\n\n", argc - 3 , argv[3]);
			return 0;
		}
		if(!strcmp(argv[4], "gap++")) 			gap_more = 1;
		else if(!strcmp(argv[4], "gap--"))	 	gap_less = 1;
		else if(!strcmp(argv[4], "vmode")) 		vmode = 1;
		else if(!strcmp(argv[4], "trainer")) 	trainer = 1;
		else {
			printf("Syntax Error : Argument %d (%s) is not valid.\n\n", argc - 2 , argv[4]);
			return 0;
		}
		// argv[5] is the output file name
	}

	if(argc >= 7) { // No moar plz
		printf("Error: I don't need %d args, one input file is enuff\n\n", argc - 1);
		printf("Usage :\n");
		printf("%s input.cue <cmd_1> <cmd_2> <cmd_3> <output.vcd>\n\n", argv[0]);
		printf("Commands are :\n");
		printf("gap++ : Adds 2 seconds to all track indexes MSF\n");
		printf("gap-- : Substracts 2 seconds to all track indexes MSF\n");
		printf("vmode : Attempts to patch the video mode to NTSC and to fix the screen position\n");
		printf("trainer : Enable cheats\n\n");
		printf("Examples :\n");
		printf("%s mygame.cue\n", argv[0]);
		printf("%s mygame.cue IMAGE0.VCD\n", argv[0]);
		printf("%s mygame.cue vmode\n", argv[0]);
		printf("%s mygame.cue gap++\n", argv[0]);
		printf("%s mygame.cue gap--\n", argv[0]);
		printf("%s mygame.cue trainer\n", argv[0]);
		printf("%s mygame.cue vmode IMAGE0.VCD\n", argv[0]);
		printf("%s mygame.cue gap++ IMAGE0.VCD\n", argv[0]);
		printf("%s mygame.cue gap-- IMAGE0.VCD\n", argv[0]);
		printf("%s mygame.cue trainer IMAGE0.VCD\n", argv[0]);
		printf("%s mygame.cue gap++ vmode\n", argv[0]);
		printf("%s mygame.cue gap++ trainer\n", argv[0]);
		printf("%s mygame.cue gap-- vmode\n", argv[0]);
		printf("%s mygame.cue gap-- trainer\n", argv[0]);
		printf("%s mygame.cue gap++ vmode IMAGE0.VCD\n", argv[0]);
		printf("%s mygame.cue gap-- vmode IMAGE0.VCD\n", argv[0]);
		printf("%s mygame.cue gap++ trainer IMAGE0.VCD\n", argv[0]);
		printf("%s mygame.cue gap-- trainer IMAGE0.VCD\n", argv[0]);
		printf("%s mygame.cue gap++ vmode trainer IMAGE0.VCD\n", argv[0]);
		printf("%s mygame.cue gap-- vmode trainer IMAGE0.VCD\n", argv[0]);
		printf("Commands and output file are optional.\n\n");
		return 0;
	}

	if(gap_more == 1 && gap_less == 1) { // User is dumb
		printf("Syntax Error : Conflicting gap++/gap-- arguments.\n\n");
		return 0;
	}

	if(debug != 0) {
		printf("CMD SWITCHES :\n");
		printf("gap_more   = %d\n", gap_more);
		printf("gap_less   = %d\n", gap_less);
		printf("vmode      = %d\n", vmode);
		printf("trainer    = %d\n\n", trainer);
	}


	if(!(file = fopen(argv[1], "rb"))) { // Open the cue sheet
		printf("Error: Cannot open %s\n\n", argv[1]);
		return 0;
	}
	fseek(file, 0, SEEK_END);
	cuesize = ftell(file);
	rewind(file);
	cuebuf = malloc(cuesize * 2);
	fread(cuebuf, cuesize, 1, file);
	fclose(file);

	ptr = strstr(cuebuf, "INDEX 01 ");
	ptr += 9;
	if((ptr[0] != '0') || (ptr[1] != '0')) {
		printf("Error: The cue sheet is not valid\n\n");
		free(cuebuf);
		return 0;
	}

	ptr = strstr(cuebuf, "FILE ");
	ptr += 5; // Jump to the BINARY name/path starting with " (it's right after "FILE ")
	if(ptr[0] != '"') {
		printf("Error: The cue sheet is not valid\n\n");
		free(cuebuf);
		return 0;
	}

	for(i = 0; i < cuesize; i++) {
		if(cuebuf[i] == '"') {
			cuebuf[i] = '\0';
		}
	}
	ptr++; // Jump to the BINARY name/path starting with "


	bin_path = malloc((strlen(ptr) + strlen(argv[1])) * 2);

	for(i = strlen(ptr); i > 0; i--) { // Does the cue have the full BINARY path ?
	  if((ptr[i] == '\\') || (ptr[i] == '/')) { // YES !
			strcpy(bin_path, ptr);
			break;
		}
	}
	if(i == 0) { // If no..
		for(i = strlen(argv[1]); i > 0; i--) { // Does the arg have the full cue path ?
		  if((argv[1][i] == '\\') || (argv[1][i] == '/')) {
		    break; // YES !
		  }
		}

		if(i == 0) {
		  // Having a filename without hierarchy is perfectly ok.
		  strcpy(bin_path, ptr);
		} else { // Here we've got the full CUE path. We're gonna use it to make the BIN path.
			strcpy(bin_path, argv[1]);
			/* Why should I use strrchr when I can do a n00ber thing ;D */
			for(i = strlen(bin_path); i > 0; i--) {
			  if((bin_path[i] == '\\') || (bin_path[i] == '/'))
			     break;
			}
			for(i = i+1; (unsigned long) i < strlen(bin_path); i++) bin_path[i] = 0x00; // How kewl is dat ?
			/* Me no liek strncat */
			i = strlen(bin_path);
			strcpy(bin_path + i, ptr);
			i = strlen(bin_path);
			if(argv[1][0] == '"') bin_path[i] = '"';
			else bin_path[i] = '\0';
		}
	}

	if(debug != 0) {
		printf("CUE Path   = %s\n", argv[1]);
		printf("BIN Path   = %s\n\n", bin_path);
	}

	headerbuf = malloc(headersize * 2);

	/*******************************************************************************************************
	********************************************************************************************************
	Below are the 3 descriptors (POPS header offset 0h to 1Eh) set up for PlayStation CD-ROM dumps
	Why I didn't use structure definitions ? Because I don't like it.
	--------------------------------------------------------------------------------------------------------
	Also note that headerbuf + 1032 to headerbuf + 1040 is the disc size in sectors :
	memcpy(headerbuf + 1032, &sector_count, 3); // Sector Count (LEHEX)
	memcpy(headerbuf + 1036, &sector_count, 3); // Sector Count (LEHEX)
	headerbuf[1032] = A
	headerbuf[1033] = B
	headerbuf[1034] = L
	headerbuf[1035] = NOP (else, the size is out of bounds)
	headerbuf[1036] = A
	headerbuf[1037] = B
	headerbuf[1038] = L
	headerbuf[1039] = NOP (else, the size is out of bounds)
	--------------------------------------------------------------------------------------------------------
	Example of a track entry (10 bytes) :
	headerbuf[30] = 0x41;	// Track Type = 41h for DATA, 01h for CDDA
	headerbuf[31] = 0x00;	// NULL
	headerbuf[32] = 0x01;	// Track Number (01h to 99h). We're at headerbuf + 32 in this example, so it's 01h for Track 01
	headerbuf[33] = 0x00;	// INDEX 00 MM
	headerbuf[34] = 0x00;	// INDEX 00 SS
	headerbuf[35] = 0x00;	// INDEX 00 FF
	headerbuf[36] = 0x00;	// NULL
	headerbuf[37] = 0x00;	// INDEX 01 MM
	headerbuf[38] = 0x02;	// INDEX 01 SS
	headerbuf[39] = 0x00;	// INDEX 01 FF
	========================================================================================================
	Alright, here are the 3 descriptors (A0, A1 and A2) :
	********************************************************************************************************
	*******************************************************************************************************/
	headerbuf[0] = 0x41;	// 1st track Type = 41h for DATA
//	headerbuf[1] = 0x00;	// NULL
	headerbuf[2] = 0xA0;	// Descriptor Identifier = A0h for DISC TYPE array
//	headerbuf[3] = 0x00;	// 1st track MM
//	headerbuf[4] = 0x00;	// 1st track SS
//	headerbuf[5] = 0x00;	// 1st track FF
//	headerbuf[6] = 0x00;	// NULL
	headerbuf[7] = 0x01;	// Track number of the 1st track = 1
	headerbuf[8] = 0x20;	// Disc Type = 20h for CD-XA001
//	headerbuf[9] = 0x00;	// NULL
//	headerbuf[10] = 0x41;	// Content Type = We'll put the last track type here later (01h = mixed with CD-DA, 41h = single DATA track)
//	headerbuf[11] = 0x00;	// NULL
	headerbuf[12] = 0xA1;	// Descriptor Identifier = A1h for CONTENT array
//	headerbuf[13] = 0x00;	// Content start MM
//	headerbuf[14] = 0x00;	// Content start SS
//	headerbuf[15] = 0x00;	// Content start FF
//	headerbuf[16] = 0x00;	// NULL
//	headerbuf[17] = 0x99;	// Track count
//	headerbuf[18] = 0x00;	// NULL
//	headerbuf[19] = 0x00;	// NULL
//	headerbuf[20] = 0x41	// 2013/05/16, v2.0 : Content Type = We'll put the last track type here later (01h = mixed with CD-DA, 41h = single DATA track)
//	headerbuf[21] = 0x00;	// NULL
	headerbuf[22] = 0xA2;	// Descriptor Identifier = A2h for LEAD-IN/LEAD-OUT array
//	headerbuf[23] = 0x00;	// Lead-In MM
//	headerbuf[24] = 0x00;	// Lead-In SS
//	headerbuf[25] = 0x00;	// Lead-In FF
//	headerbuf[26] = 0x00;	// NULL
//	headerbuf[27] = 0x79;	// Lead-Out MM
//	headerbuf[28] = 0x59;	// Lead-Out SS
//	headerbuf[29] = 0x74;	// Lead-Out FF
	/*******************************************************************************************************
	*******************************************************************************************************/

	for(i = 0; i < cuesize; i++) { // Since I've nulled some chars in the BIN handler, here I prelocate the TRACK 01 string so the track type substring can be found
		if(cuebuf[i] == 'T' && cuebuf[i+1] == 'R' && cuebuf[i+2] == 'A' && cuebuf[i+3] == 'C' && cuebuf[i+4] == 'K' && cuebuf[i+5] == ' ' && cuebuf[i+6] == '0' && cuebuf[i+7] == '1') break;
	}

	ptr = strstr(cuebuf + i, "TRACK 01 MODE2/2352"); // Ought be
	if(ptr != NULL) {
		if(debug != 0) printf("Disc Type Check : Is MODE2/2352\n");
	} else { // 2013/05/16, v2.0 : Not MODE2/2352, tell the user and terminate
		printf("Error: Looks like your game dump is not MODE2/2352, or the cue is invalid.\n\n");
		free(cuebuf);
		free(bin_path);
		free(headerbuf);
		return 0;
	}

	for(i = 0; i < cuesize; i++) {
		/* Clean out some crap in the cuebuf */
		if(cuebuf[i] == ':') cuebuf[i] = '\0';
		if(cuebuf[i] == 0x0D) cuebuf[i] = '\0';
		if(cuebuf[i] == 0x0A) cuebuf[i] = '\0';
		/* Count stuff in the cue */
		if(cuebuf[i] == 'T' && cuebuf[i+1] == 'R' && cuebuf[i+2] == 'A' && cuebuf[i+3] == 'C' && cuebuf[i+4] == 'K' && cuebuf[i+5] == ' ') track_count++;
		if(cuebuf[i] == 'I' && cuebuf[i+1] == 'N' && cuebuf[i+2] == 'D' && cuebuf[i+3] == 'E' && cuebuf[i+4] == 'X' && cuebuf[i+5] == ' ' && cuebuf[i+6] == '0' && cuebuf[i+7] == '1') index1_count++;
		if(cuebuf[i] == 'I' && cuebuf[i+1] == 'N' && cuebuf[i+2] == 'D' && cuebuf[i+3] == 'E' && cuebuf[i+4] == 'X' && cuebuf[i+5] == ' ' && cuebuf[i+6] == '0' && cuebuf[i+7] == '0') index0_count++;
		if(cuebuf[i] == 'B' && cuebuf[i+1] == 'I' && cuebuf[i+2] == 'N' && cuebuf[i+3] == 'A' && cuebuf[i+4] == 'R' && cuebuf[i+5] == 'Y') binary_count++;
		if(cuebuf[i] == 'W' && cuebuf[i+1] == 'A' && cuebuf[i+2] == 'V' && cuebuf[i+3] == 'E') wave_count++;
		if(cuebuf[i] == 'P' && cuebuf[i+1] == 'R' && cuebuf[i+2] == 'E' && cuebuf[i+3] == 'G' && cuebuf[i+4] == 'A' && cuebuf[i+5] == 'P') pregap_count++;
		if(cuebuf[i] == 'P' && cuebuf[i+1] == 'O' && cuebuf[i+2] == 'S' && cuebuf[i+3] == 'T' && cuebuf[i+4] == 'G' && cuebuf[i+5] == 'A' && cuebuf[i+6] == 'P') postgap_count++;
	}

	if(debug != 0) {
		printf("pregap_count    = %d\n", pregap_count);
		printf("postgap_count   = %d\n", postgap_count);
		printf("track_count     = %d\n", track_count);
		printf("index0_count    = %d\n", index0_count);
		printf("index1_count    = %d\n", index1_count);
		printf("binary_count    = %d\n\n", binary_count);
	}
	if(binary_count == 0) { // WTF ?
		printf("Error: Unstandard cue sheet\n\n");
		free(cuebuf);
		free(bin_path);
		free(headerbuf);
		return 0;
	}
	if((track_count == 0) || (track_count != index1_count)) { // Huh ?
		printf("Error : Cannot count tracks\n\n");
		free(cuebuf);
		free(bin_path);
		free(headerbuf);
		return 0;
	}
	if(binary_count != 1 || wave_count != 0) { // I urd u liek warez^^
		printf("Error: Cue sheets of splitted dumps aren't supported\n\n");
		free(cuebuf);
		free(bin_path);
		free(headerbuf);
		return 0;
	}

	if(pregap_count == 1 && postgap_count == 0) { // Don't offer to fix the dump if more than 1 pregap was found or cue has postgaps
		if(batch == 0) {
			for(i = pregap_count; i > 0; i--) {
				i++;
				printf("Warning : The input file seems to be a CDRWIN cue sheet...\n");
				printf("          Would you like to fix the disc image (Yes or No) ? ");
				scanf("%s", answer);
				if(!strcmp(answer, "Yes") || !strcmp(answer, "yes") || !strcmp(answer, "y") || !strcmp(answer, "Y")) {
					fix_CDRWIN = 1;
					printf("\n");
					break;
				}
				if(!strcmp(answer, "No") || !strcmp(answer, "no") || !strcmp(answer, "n") || !strcmp(answer, "N")) {
					printf("\n");
					break;
				}
			}
		} else { // Batch mode is ON, so don't prompt the user...
			printf("Warning : The input file seems to be a CDRWIN cue sheet\n");
			printf("          A pregap will be inserted in the output file...\n\n");
			fix_CDRWIN = 1; //... And turn the CDRWIN  fix on
		}
	}

	for(i = 0; i < track_count; i++) {
		header_ptr += 10; // Put the pointer at the start of the correct track entry

		for(cue_ptr = 0; cue_ptr < cuesize; cue_ptr++) {
			if(cuebuf[cue_ptr] == 'T' && cuebuf[cue_ptr+1] == 'R' && cuebuf[cue_ptr+2] == 'A' && cuebuf[cue_ptr+3] == 'C' && cuebuf[cue_ptr+4] == 'K' && cuebuf[cue_ptr+5] == ' ') {
				cuebuf[cue_ptr] = '\0';
				cuebuf[cue_ptr + 1] = '\0';
				cuebuf[cue_ptr + 2] = '\0';
				cuebuf[cue_ptr + 3] = '\0';
				cuebuf[cue_ptr + 4] = '\0';
				cuebuf[cue_ptr + 5] = '\0';

				/* Track Type : 41h = DATA Track / 01h CDDA Track */
				if(cuebuf[cue_ptr + 13] == '2' || cuebuf[cue_ptr + 13] == '1' || cuebuf[cue_ptr + 13] != 'O') {
					headerbuf[10] = 0x41; 			// Assume DATA Track
					headerbuf[20] = 0x41; 			// 2013/05/16, v2.0 : Assume DATA Track
					headerbuf[header_ptr] = 0x41; 	// Assume DATA Track
				}
				if(cuebuf[cue_ptr + 13] == 'O') {
					headerbuf[10] = 0x01; 			// Assume AUDIO Track
					headerbuf[20] = 0x01; 			// 2013/05/16, v2.0 : Assume AUDIO Track
					headerbuf[header_ptr] = 0x01; 	// Assume AUDIO Track
				}

				headerbuf[header_ptr + 2] = ((cuebuf[cue_ptr + 6] - 48) * 16) + (cuebuf[cue_ptr + 7] - 48);	// Track Number
				headerbuf[17] = ((cuebuf[cue_ptr + 6] - 48) * 16) + (cuebuf[cue_ptr + 7] - 48);				// Number of track entries
				break;
			}
		}

		for(cue_ptr = 0; cue_ptr < cuesize; cue_ptr++) {
			if(cuebuf[cue_ptr] == 'I' && cuebuf[cue_ptr+1] == 'N' && cuebuf[cue_ptr+2] == 'D' && cuebuf[cue_ptr+3] == 'E' && cuebuf[cue_ptr+4] == 'X' && cuebuf[cue_ptr+5] == ' ' && cuebuf[cue_ptr+6] == '0' && cuebuf[cue_ptr+7] == '0') {
				gap_ptr = cue_ptr; // Memoryze the location of the INDEX 00 entry
				cuebuf[cue_ptr] = '\0';
				cuebuf[cue_ptr + 1] = '\0';
				cuebuf[cue_ptr + 2] = '\0';
				cuebuf[cue_ptr + 3] = '\0';
				cuebuf[cue_ptr + 4] = '\0';
				cuebuf[cue_ptr + 5] = '\0';
				cuebuf[cue_ptr + 6] = '\0';
				cuebuf[cue_ptr + 7] = '\0';
			}
			if(cuebuf[cue_ptr] == 'I' && cuebuf[cue_ptr+1] == 'N' && cuebuf[cue_ptr+2] == 'D' && cuebuf[cue_ptr+3] == 'E' && cuebuf[cue_ptr+4] == 'X' && cuebuf[cue_ptr+5] == ' ' && cuebuf[cue_ptr+6] == '0' && cuebuf[cue_ptr+7] == '1') {
				cuebuf[cue_ptr] = '\0';
				cuebuf[cue_ptr + 1] = '\0';
				cuebuf[cue_ptr + 2] = '\0';
				cuebuf[cue_ptr + 3] = '\0';
				cuebuf[cue_ptr + 4] = '\0';
				cuebuf[cue_ptr + 5] = '\0';
				cuebuf[cue_ptr + 6] = '\0';
				cuebuf[cue_ptr + 7] = '\0';

				m = ((cuebuf[cue_ptr + 9] - 48) * 16) + (cuebuf[cue_ptr + 10] - 48);
				s = ((cuebuf[cue_ptr + 12] - 48) * 16) + (cuebuf[cue_ptr + 13] - 48);
				f = ((cuebuf[cue_ptr + 15] - 48) * 16) + (cuebuf[cue_ptr + 16] - 48);

				if(daTrack_ptr == 0 && headerbuf[10] == 0x01 && gap_ptr != 0) { // Targets the first AUDIO track INDEX 00 MSF
					daTrack_ptr = (((((cuebuf[gap_ptr + 9] - 48) * 10) + (cuebuf[gap_ptr + 10] - 48)) * 4500) + ((((cuebuf[gap_ptr + 12] - 48) * 10) + (cuebuf[gap_ptr + 13] - 48)) * 75) + (((cuebuf[gap_ptr + 15] - 48) * 10) + (cuebuf[gap_ptr + 16] - 48))) * sectorsize;
					if(debug != 0) {
						printf("daTrack_ptr hit on TRACK %02d AUDIO INDEX 00 MSF minus 2 seconds !\n", i + 1);
						printf("Current daTrack_ptr = %d (%Xh)\n", daTrack_ptr, daTrack_ptr);
						printf("daTrack_ptr LBA     = %d (%Xh)\n\n", daTrack_ptr / sectorsize, daTrack_ptr / sectorsize);
					}
				} else if(daTrack_ptr == 0 && headerbuf[10] == 0x01 && gap_ptr == 0) { // Targets the first AUDIO track INDEX 01 MSF
					daTrack_ptr = (((((cuebuf[cue_ptr + 9] - 48) * 10) + (cuebuf[cue_ptr + 10] - 48)) * 4500) + ((((cuebuf[cue_ptr + 12] - 48) * 10) + (cuebuf[cue_ptr + 13] - 48)) * 75) + (((cuebuf[cue_ptr + 15] - 48) * 10) + (cuebuf[cue_ptr + 16] - 48))) * sectorsize;
					if(debug != 0) {
						printf("daTrack_ptr hit on TRACK %02d AUDIO INDEX 01 MSF minus 2 seconds !\n", i + 1);
						printf("Current daTrack_ptr = %d (%Xh)\n", daTrack_ptr, daTrack_ptr);
						printf("daTrack_ptr LBA     = %d (%Xh)\n\n", daTrack_ptr / sectorsize, daTrack_ptr / sectorsize);
					}
				} else if(daTrack_ptr == 0 && headerbuf[10] == 0x41 && i == track_count - 1) { // Targets the EOF (if no audio track was found)
					noCDDA = 1; // 2013/05/16 - v2.0
					if(debug != 0) {
						printf("No CD-DA track found after TRACK %02d.\n", i + 1);
						printf("daTrack_ptr will be set to the EOF.\n\n"); // 2013/04/22 - v1.2 : Yeah, it WILL.
					}
				}

				/* Since not all cue sheet have INDEX 00 entries, we dupe the INDEX 01 MSF to the INDEX 00 MSF field... */
				headerbuf[header_ptr + 3] = m;		// Absolute MM (INDEX 00)
				headerbuf[header_ptr + 4] = s;		// Absolute SS (INDEX 00)
				headerbuf[header_ptr + 5] = f;		// Absolute FF (INDEX 00)

				headerbuf[header_ptr + 7] = m;		// Relative MM (INDEX 01)
				headerbuf[header_ptr + 8] = s;		// Relative SS (INDEX 01)
				headerbuf[header_ptr + 9] = f;		// Relative FF (INDEX 01)

				/* ... If INDEX 00 was found, then we register it's MSF to the INDEX 00 MSF field 	*/
				if(gap_ptr != 0) { // Pregap (INDEX 00) was found before
					m = ((cuebuf[gap_ptr + 9] - 48) * 16) + (cuebuf[gap_ptr + 10] - 48);
					s = ((cuebuf[gap_ptr + 12] - 48) * 16) + (cuebuf[gap_ptr + 13] - 48);
					f = ((cuebuf[gap_ptr + 15] - 48) * 16) + (cuebuf[gap_ptr + 16] - 48);
					headerbuf[header_ptr + 3] = m;		// Absolute MM (INDEX 00)
					headerbuf[header_ptr + 4] = s;		// Absolute SS (INDEX 00)
					headerbuf[header_ptr + 5] = f;		// Absolute FF (INDEX 00)
				}



				/* 2013/05/16 - v2.0, Enough already, let's fix this shit once and for all the NASTY WAY */

				/* INDEX 00 : UNCONDITIONAL + 2 SEC */
				if((i != 0) && (headerbuf[header_ptr + 4] == 0x08 || headerbuf[header_ptr + 4] == 0x09 || headerbuf[header_ptr + 4] == 0x18 || headerbuf[header_ptr + 4] == 0x19 || headerbuf[header_ptr + 4] == 0x28 || headerbuf[header_ptr + 4] == 0x29 || headerbuf[header_ptr + 4] == 0x38 || headerbuf[header_ptr + 4] == 0x39 || headerbuf[header_ptr + 4] == 0x48 || headerbuf[header_ptr + 4] == 0x49) && (headerbuf[header_ptr + 4] != 0x58 || headerbuf[header_ptr + 4] != 0x59)) {
					headerbuf[header_ptr + 4] += 8; // HEX -> DEC (+ 2 SEC)
				} else if((i != 0) && (headerbuf[header_ptr + 4] == 0x58 || headerbuf[header_ptr + 4] == 0x59)) {
					if(headerbuf[header_ptr + 4] == 0x58) headerbuf[header_ptr + 4] = 0x00; // HEX -> DEC (+ 2 SEC)
					if(headerbuf[header_ptr + 4] == 0x59) headerbuf[header_ptr + 4] = 0x01; // HEX -> DEC (+ 2 SEC)
					if(headerbuf[header_ptr + 3] == 0x09) headerbuf[header_ptr + 3] = 0x10; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 3] == 0x19) headerbuf[header_ptr + 3] = 0x20; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 3] == 0x29) headerbuf[header_ptr + 3] = 0x30; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 3] == 0x39) headerbuf[header_ptr + 3] = 0x40; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 3] == 0x49) headerbuf[header_ptr + 3] = 0x50; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 3] == 0x59) headerbuf[header_ptr + 3] = 0x60; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 3] == 0x69) headerbuf[header_ptr + 3] = 0x70; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 3] == 0x79) headerbuf[header_ptr + 3] = 0x80; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 3] == 0x89) headerbuf[header_ptr + 3] = 0x90; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 3] != 0x09 && headerbuf[header_ptr + 3] != 0x19 && headerbuf[header_ptr + 3] != 0x29 && headerbuf[header_ptr + 3] != 0x39 && headerbuf[header_ptr + 3] != 0x49 && headerbuf[header_ptr + 3] != 0x59 && headerbuf[header_ptr + 3] != 0x69 && headerbuf[header_ptr + 3] != 0x79 && headerbuf[header_ptr + 3] != 0x89) headerbuf[header_ptr + 3] += 1;
				} else if(i != 0) {
					headerbuf[header_ptr + 4] += 2; // + 2 SEC
				}


				/* INDEX 01 : UNCONDITIONAL + 2 SEC */
				if((i != 0) && (headerbuf[header_ptr + 8] == 0x08 || headerbuf[header_ptr + 8] == 0x09 || headerbuf[header_ptr + 8] == 0x18 || headerbuf[header_ptr + 8] == 0x19 || headerbuf[header_ptr + 8] == 0x28 || headerbuf[header_ptr + 8] == 0x29 || headerbuf[header_ptr + 8] == 0x38 || headerbuf[header_ptr + 8] == 0x39 || headerbuf[header_ptr + 8] == 0x48 || headerbuf[header_ptr + 8] == 0x49) && (headerbuf[header_ptr + 8] != 0x58 || headerbuf[header_ptr + 8] != 0x59)) {
					headerbuf[header_ptr + 8] += 8; // HEX -> DEC (+ 2 SEC)
				} else if((i != 0) && (headerbuf[header_ptr + 8] == 0x58 || headerbuf[header_ptr + 8] == 0x59)) {
					if(headerbuf[header_ptr + 8] == 0x58) headerbuf[header_ptr + 8] = 0x00; // HEX -> DEC (+ 2 SEC)
					if(headerbuf[header_ptr + 8] == 0x59) headerbuf[header_ptr + 8] = 0x01; // HEX -> DEC (+ 2 SEC)
					if(headerbuf[header_ptr + 7] == 0x09) headerbuf[header_ptr + 7] = 0x10; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 7] == 0x19) headerbuf[header_ptr + 7] = 0x20; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 7] == 0x29) headerbuf[header_ptr + 7] = 0x30; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 7] == 0x39) headerbuf[header_ptr + 7] = 0x40; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 7] == 0x49) headerbuf[header_ptr + 7] = 0x50; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 7] == 0x59) headerbuf[header_ptr + 7] = 0x60; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 7] == 0x69) headerbuf[header_ptr + 7] = 0x70; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 7] == 0x79) headerbuf[header_ptr + 7] = 0x80; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 7] == 0x89) headerbuf[header_ptr + 7] = 0x90; // HEX -> DEC (+ 1 MIN)
					if(headerbuf[header_ptr + 7] != 0x09 && headerbuf[header_ptr + 7] != 0x19 && headerbuf[header_ptr + 7] != 0x29 && headerbuf[header_ptr + 7] != 0x39 && headerbuf[header_ptr + 7] != 0x49 && headerbuf[header_ptr + 7] != 0x59 && headerbuf[header_ptr + 7] != 0x69 && headerbuf[header_ptr + 7] != 0x79 && headerbuf[header_ptr + 7] != 0x89) headerbuf[header_ptr + 7] += 1;
				} else { // Also for the 1st track
					headerbuf[header_ptr + 8] += 2; // + 2 SEC
				}

				/* 2013/05/16 - v2.0, End of the unconditional +2sec fix */



				/* 2013/05/16 - v2.0, And redo + 2 SEC for fixing CDRWIN dumps */

				if(fix_CDRWIN == 1) {
					/* INDEX 00 : + 2 SEC */
					if((i != 0) && (headerbuf[header_ptr + 4] == 0x08 || headerbuf[header_ptr + 4] == 0x09 || headerbuf[header_ptr + 4] == 0x18 || headerbuf[header_ptr + 4] == 0x19 || headerbuf[header_ptr + 4] == 0x28 || headerbuf[header_ptr + 4] == 0x29 || headerbuf[header_ptr + 4] == 0x38 || headerbuf[header_ptr + 4] == 0x39 || headerbuf[header_ptr + 4] == 0x48 || headerbuf[header_ptr + 4] == 0x49) && (headerbuf[header_ptr + 4] != 0x58 || headerbuf[header_ptr + 4] != 0x59)) {
						headerbuf[header_ptr + 4] += 8; // HEX -> DEC (+ 2 SEC)
					} else if((i != 0) && (headerbuf[header_ptr + 4] == 0x58 || headerbuf[header_ptr + 4] == 0x59)) {
						if(headerbuf[header_ptr + 4] == 0x58) headerbuf[header_ptr + 4] = 0x00; // HEX -> DEC (+ 2 SEC)
						if(headerbuf[header_ptr + 4] == 0x59) headerbuf[header_ptr + 4] = 0x01; // HEX -> DEC (+ 2 SEC)
						if(headerbuf[header_ptr + 3] == 0x09) headerbuf[header_ptr + 3] = 0x10; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x19) headerbuf[header_ptr + 3] = 0x20; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x29) headerbuf[header_ptr + 3] = 0x30; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x39) headerbuf[header_ptr + 3] = 0x40; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x49) headerbuf[header_ptr + 3] = 0x50; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x59) headerbuf[header_ptr + 3] = 0x60; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x69) headerbuf[header_ptr + 3] = 0x70; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x79) headerbuf[header_ptr + 3] = 0x80; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x89) headerbuf[header_ptr + 3] = 0x90; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] != 0x09 && headerbuf[header_ptr + 3] != 0x19 && headerbuf[header_ptr + 3] != 0x29 && headerbuf[header_ptr + 3] != 0x39 && headerbuf[header_ptr + 3] != 0x49 && headerbuf[header_ptr + 3] != 0x59 && headerbuf[header_ptr + 3] != 0x69 && headerbuf[header_ptr + 3] != 0x79 && headerbuf[header_ptr + 3] != 0x89) headerbuf[header_ptr + 3] += 1;
					} else if(i != 0) {
						headerbuf[header_ptr + 4] += 2; // + 2 SEC
					}
					/* INDEX 01 : + 2 SEC */
					if((i != 0) && (headerbuf[header_ptr + 8] == 0x08 || headerbuf[header_ptr + 8] == 0x09 || headerbuf[header_ptr + 8] == 0x18 || headerbuf[header_ptr + 8] == 0x19 || headerbuf[header_ptr + 8] == 0x28 || headerbuf[header_ptr + 8] == 0x29 || headerbuf[header_ptr + 8] == 0x38 || headerbuf[header_ptr + 8] == 0x39 || headerbuf[header_ptr + 8] == 0x48 || headerbuf[header_ptr + 8] == 0x49) && (headerbuf[header_ptr + 8] != 0x58 || headerbuf[header_ptr + 8] != 0x59)) {
						headerbuf[header_ptr + 8] += 8; // HEX -> DEC (+ 2 SEC)
					} else if((i != 0) && (headerbuf[header_ptr + 8] == 0x58 || headerbuf[header_ptr + 8] == 0x59)) {
						if(headerbuf[header_ptr + 8] == 0x58) headerbuf[header_ptr + 8] = 0x00; // HEX -> DEC (+ 2 SEC)
						if(headerbuf[header_ptr + 8] == 0x59) headerbuf[header_ptr + 8] = 0x01; // HEX -> DEC (+ 2 SEC)
						if(headerbuf[header_ptr + 7] == 0x09) headerbuf[header_ptr + 7] = 0x10; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x19) headerbuf[header_ptr + 7] = 0x20; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x29) headerbuf[header_ptr + 7] = 0x30; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x39) headerbuf[header_ptr + 7] = 0x40; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x49) headerbuf[header_ptr + 7] = 0x50; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x59) headerbuf[header_ptr + 7] = 0x60; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x69) headerbuf[header_ptr + 7] = 0x70; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x79) headerbuf[header_ptr + 7] = 0x80; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x89) headerbuf[header_ptr + 7] = 0x90; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] != 0x09 && headerbuf[header_ptr + 7] != 0x19 && headerbuf[header_ptr + 7] != 0x29 && headerbuf[header_ptr + 7] != 0x39 && headerbuf[header_ptr + 7] != 0x49 && headerbuf[header_ptr + 7] != 0x59 && headerbuf[header_ptr + 7] != 0x69 && headerbuf[header_ptr + 7] != 0x79 && headerbuf[header_ptr + 7] != 0x89) headerbuf[header_ptr + 7] += 1;
					} else if(i != 0) {
						headerbuf[header_ptr + 8] += 2; // + 2 SEC
					}
				}

				/* 2013/05/16 - v2.0, End of the CDRWIN +2sec fix */



				/* 2013/05/16 - v2.0, gap++ */

				if(gap_more == 1) {
					/* INDEX 00 : + 2 SEC */
					if((i != 0) && (headerbuf[header_ptr + 4] == 0x08 || headerbuf[header_ptr + 4] == 0x09 || headerbuf[header_ptr + 4] == 0x18 || headerbuf[header_ptr + 4] == 0x19 || headerbuf[header_ptr + 4] == 0x28 || headerbuf[header_ptr + 4] == 0x29 || headerbuf[header_ptr + 4] == 0x38 || headerbuf[header_ptr + 4] == 0x39 || headerbuf[header_ptr + 4] == 0x48 || headerbuf[header_ptr + 4] == 0x49) && (headerbuf[header_ptr + 4] != 0x58 || headerbuf[header_ptr + 4] != 0x59)) {
						headerbuf[header_ptr + 4] += 8; // HEX -> DEC (+ 2 SEC)
					} else if((i != 0) && (headerbuf[header_ptr + 4] == 0x58 || headerbuf[header_ptr + 4] == 0x59)) {
						if(headerbuf[header_ptr + 4] == 0x58) headerbuf[header_ptr + 4] = 0x00; // HEX -> DEC (+ 2 SEC)
						if(headerbuf[header_ptr + 4] == 0x59) headerbuf[header_ptr + 4] = 0x01; // HEX -> DEC (+ 2 SEC)
						if(headerbuf[header_ptr + 3] == 0x09) headerbuf[header_ptr + 3] = 0x10; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x19) headerbuf[header_ptr + 3] = 0x20; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x29) headerbuf[header_ptr + 3] = 0x30; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x39) headerbuf[header_ptr + 3] = 0x40; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x49) headerbuf[header_ptr + 3] = 0x50; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x59) headerbuf[header_ptr + 3] = 0x60; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x69) headerbuf[header_ptr + 3] = 0x70; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x79) headerbuf[header_ptr + 3] = 0x80; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x89) headerbuf[header_ptr + 3] = 0x90; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 3] != 0x09 && headerbuf[header_ptr + 3] != 0x19 && headerbuf[header_ptr + 3] != 0x29 && headerbuf[header_ptr + 3] != 0x39 && headerbuf[header_ptr + 3] != 0x49 && headerbuf[header_ptr + 3] != 0x59 && headerbuf[header_ptr + 3] != 0x69 && headerbuf[header_ptr + 3] != 0x79 && headerbuf[header_ptr + 3] != 0x89) headerbuf[header_ptr + 3] += 1;
					} else if(i != 0) {
						headerbuf[header_ptr + 4] += 2; // + 2 SEC
					}
					/* INDEX 01 : + 2 SEC */
					if((i != 0) && (headerbuf[header_ptr + 8] == 0x08 || headerbuf[header_ptr + 8] == 0x09 || headerbuf[header_ptr + 8] == 0x18 || headerbuf[header_ptr + 8] == 0x19 || headerbuf[header_ptr + 8] == 0x28 || headerbuf[header_ptr + 8] == 0x29 || headerbuf[header_ptr + 8] == 0x38 || headerbuf[header_ptr + 8] == 0x39 || headerbuf[header_ptr + 8] == 0x48 || headerbuf[header_ptr + 8] == 0x49) && (headerbuf[header_ptr + 8] != 0x58 || headerbuf[header_ptr + 8] != 0x59)) {
						headerbuf[header_ptr + 8] += 8; // HEX -> DEC (+ 2 SEC)
					} else if((i != 0) && (headerbuf[header_ptr + 8] == 0x58 || headerbuf[header_ptr + 8] == 0x59)) {
						if(headerbuf[header_ptr + 8] == 0x58) headerbuf[header_ptr + 8] = 0x00; // HEX -> DEC (+ 2 SEC)
						if(headerbuf[header_ptr + 8] == 0x59) headerbuf[header_ptr + 8] = 0x01; // HEX -> DEC (+ 2 SEC)
						if(headerbuf[header_ptr + 7] == 0x09) headerbuf[header_ptr + 7] = 0x10; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x19) headerbuf[header_ptr + 7] = 0x20; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x29) headerbuf[header_ptr + 7] = 0x30; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x39) headerbuf[header_ptr + 7] = 0x40; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x49) headerbuf[header_ptr + 7] = 0x50; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x59) headerbuf[header_ptr + 7] = 0x60; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x69) headerbuf[header_ptr + 7] = 0x70; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x79) headerbuf[header_ptr + 7] = 0x80; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x89) headerbuf[header_ptr + 7] = 0x90; // HEX -> DEC (+ 1 MIN)
						if(headerbuf[header_ptr + 7] != 0x09 && headerbuf[header_ptr + 7] != 0x19 && headerbuf[header_ptr + 7] != 0x29 && headerbuf[header_ptr + 7] != 0x39 && headerbuf[header_ptr + 7] != 0x49 && headerbuf[header_ptr + 7] != 0x59 && headerbuf[header_ptr + 7] != 0x69 && headerbuf[header_ptr + 7] != 0x79 && headerbuf[header_ptr + 7] != 0x89) headerbuf[header_ptr + 7] += 1;
					} else if(i != 0) {
						headerbuf[header_ptr + 8] += 2; // + 2 SEC
					}
				}

				/* 2013/05/16 - v2.0, End of the gap++ code */



				/* 2013/05/16 - v2.0, gap-- */

				if(gap_less == 1) {
					/* INDEX 00 : - 2 SEC */
					if((i != 0) && (headerbuf[header_ptr + 4] == 0x10 || headerbuf[header_ptr + 4] == 0x11 || headerbuf[header_ptr + 4] == 0x20 || headerbuf[header_ptr + 4] == 0x21 || headerbuf[header_ptr + 4] == 0x30 || headerbuf[header_ptr + 4] == 0x31 || headerbuf[header_ptr + 4] == 0x40 || headerbuf[header_ptr + 4] == 0x41 || headerbuf[header_ptr + 4] == 0x50 || headerbuf[header_ptr + 4] == 0x51) && (headerbuf[header_ptr + 4] != 0x00 || headerbuf[header_ptr + 4] != 0x01)) {
						headerbuf[header_ptr + 4] += 8; // HEX -> DEC (- 2 SEC)
					} else if((i != 0) && (headerbuf[header_ptr + 4] == 0x00 || headerbuf[header_ptr + 4] == 0x01)) {
						if(headerbuf[header_ptr + 4] == 0x00) headerbuf[header_ptr + 4] = 0x58; // HEX -> DEC (- 2 SEC)
						if(headerbuf[header_ptr + 4] == 0x01) headerbuf[header_ptr + 4] = 0x59; // HEX -> DEC (- 2 SEC)
						if(headerbuf[header_ptr + 3] == 0x09) headerbuf[header_ptr + 3] = 0x10; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x19) headerbuf[header_ptr + 3] = 0x20; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x29) headerbuf[header_ptr + 3] = 0x30; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x39) headerbuf[header_ptr + 3] = 0x40; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x49) headerbuf[header_ptr + 3] = 0x50; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x59) headerbuf[header_ptr + 3] = 0x60; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x69) headerbuf[header_ptr + 3] = 0x70; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x79) headerbuf[header_ptr + 3] = 0x80; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 3] == 0x89) headerbuf[header_ptr + 3] = 0x90; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 3] != 0x10 && headerbuf[header_ptr + 3] != 0x20 && headerbuf[header_ptr + 3] != 0x30 && headerbuf[header_ptr + 3] != 0x40 && headerbuf[header_ptr + 3] != 0x50 && headerbuf[header_ptr + 3] != 0x60 && headerbuf[header_ptr + 3] != 0x70 && headerbuf[header_ptr + 3] != 0x80 && headerbuf[header_ptr + 3] != 0x90) headerbuf[header_ptr + 3] -= 1;
					} else if(i != 0) {
						headerbuf[header_ptr + 4] -= 2; // - 2 SEC
					}
					/* INDEX 01 : - 2 SEC */
					if((i != 0) && (headerbuf[header_ptr + 8] == 0x10 || headerbuf[header_ptr + 8] == 0x11 || headerbuf[header_ptr + 8] == 0x20 || headerbuf[header_ptr + 8] == 0x21 || headerbuf[header_ptr + 8] == 0x30 || headerbuf[header_ptr + 8] == 0x31 || headerbuf[header_ptr + 8] == 0x40 || headerbuf[header_ptr + 8] == 0x41 || headerbuf[header_ptr + 8] == 0x50 || headerbuf[header_ptr + 8] == 0x51) && (headerbuf[header_ptr + 8] != 0x00 || headerbuf[header_ptr + 8] != 0x01)) {
						headerbuf[header_ptr + 8] += 8; // HEX -> DEC (- 2 SEC)
					} else if((i != 0) && (headerbuf[header_ptr + 8] == 0x00 || headerbuf[header_ptr + 8] == 0x01)) {
						if(headerbuf[header_ptr + 8] == 0x00) headerbuf[header_ptr + 8] = 0x58; // HEX -> DEC (- 2 SEC)
						if(headerbuf[header_ptr + 8] == 0x01) headerbuf[header_ptr + 8] = 0x59; // HEX -> DEC (- 2 SEC)
						if(headerbuf[header_ptr + 7] == 0x10) headerbuf[header_ptr + 7] = 0x09; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x20) headerbuf[header_ptr + 7] = 0x19; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x30) headerbuf[header_ptr + 7] = 0x29; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x40) headerbuf[header_ptr + 7] = 0x39; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x50) headerbuf[header_ptr + 7] = 0x49; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x60) headerbuf[header_ptr + 7] = 0x59; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x70) headerbuf[header_ptr + 7] = 0x69; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x80) headerbuf[header_ptr + 7] = 0x79; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 7] == 0x90) headerbuf[header_ptr + 7] = 0x89; // HEX -> DEC (- 1 MIN)
						if(headerbuf[header_ptr + 7] != 0x10 && headerbuf[header_ptr + 7] != 0x20 && headerbuf[header_ptr + 7] != 0x30 && headerbuf[header_ptr + 7] != 0x40 && headerbuf[header_ptr + 7] != 0x50 && headerbuf[header_ptr + 7] != 0x60 && headerbuf[header_ptr + 7] != 0x70 && headerbuf[header_ptr + 7] != 0x80 && headerbuf[header_ptr + 7] != 0x90) headerbuf[header_ptr + 7] -= 1;
					} else if(i != 0) {
						headerbuf[header_ptr + 8] -= 2; // - 2 SEC
					}
				}

				/* 2013/05/16 - v2.0, End of the gap-- code */


				gap_ptr = 0; // Reset the integer that points to the last NULLed INDEX 00 entry of the cue sheet

				break;
			}
		}
	}
	free(cuebuf);

	if(GetLeadOut(headerbuf) != 0) {
		free(bin_path);
		free(headerbuf);
		return 0;
	}

	if(noCDDA == 1) daTrack_ptr = bin_size; // 2013/04/22 - v1.2 : Set it now. If no CDDA track was found in the game dump, the NTSC patcher will proceed in scanning the whole BIN.
	if(noCDDA == 1 && debug != 0) {			// 2013/05/16 - v2.0 : dbg please
		printf("Current daTrack_ptr     = %d (%Xh)\n", daTrack_ptr, daTrack_ptr);
		printf("daTrack_ptr LBA         = %d (%Xh)\n\n", daTrack_ptr / sectorsize, daTrack_ptr / sectorsize);
	}

	memcpy(headerbuf + 1032, &sector_count, 4); // Sector Count (LEHEX)
	memcpy(headerbuf + 1036, &sector_count, 4); // Sector Count (LEHEX)

	headerbuf[1024] = 0x6B;
	headerbuf[1025] = 0x48;
	headerbuf[1026] = 0x6E;
	headerbuf[1027] = 0x20;	// 2013/05/16 - v2.0 : CUE2POPS ver ident

	/* 2 user arguments : no command, output file is user argument 2 */
	if(gap_more == 0 && gap_less == 0 && vmode == 0 && trainer == 0 && argc == 3) {
		printf("Saving the virtual CD-ROM image. Please wait...\n");
		if(!(file = fopen(argv[2], "wb"))) {
			printf("Error : Cannot write to %s\n\n", argv[2]);
			free(bin_path);
			free(headerbuf);
			return 0;
		}
		fwrite(headerbuf, 1, headersize, file);
		fclose(file);
		free(headerbuf);


		if(!(file = fopen(argv[2], "ab+"))) {
			printf("Error : Cannot write to %s\n\n", argv[2]);
			free(bin_path);
			return 0;
		}

		if(!(bin_file = fopen(bin_path, "rb"))) {
			printf("Error: Cannot open %s\n\n", bin_path);
			free(bin_path);
			return 0;
		}
		free(bin_path);

		for(i = 0; i < bin_size; i += headersize) {
			if(fix_CDRWIN == 1 && (i + headersize >= daTrack_ptr)) {
				if(debug != 0) printf("Padding the CDRWIN dump inside of the virtual CD-ROM image...");
				fread(outbuf, headersize - (i + headersize - daTrack_ptr), 1, bin_file);
				fwrite(outbuf, headersize - (i + headersize - daTrack_ptr), 1, file);
				char padding[(150 * sectorsize) * 2];
				fwrite(padding, 150 * sectorsize, 1, file);
				fread(outbuf, headersize - (headersize - (i + headersize - daTrack_ptr)), 1, bin_file);
				fwrite(outbuf, headersize - (headersize - (i + headersize - daTrack_ptr)), 1, file);
				fix_CDRWIN = 0;
				if(debug != 0) {
					printf(" Done.\n");
					printf("Continuing...\n");
				}
				//if(vmode == 1) printf("----------------------------------------------------------------------------------\n");
			} else {
				if(vmode == 1 && i == 0) {
					printf("----------------------------------------------------------------------------------\n");
					printf("NTSC Patcher is ON\n");
					printf("----------------------------------------------------------------------------------\n");
				}
				fread(outbuf, headersize, 1, bin_file);
				if(i == 0) GameIdentifier(outbuf);
				if(GameTitle >= 0 && GameHasCheats == 1 && trainer == 1 && i == 0) {
					printf("GameTrainer is ON\n");
					printf("----------------------------------------------------------------------------------\n");
				}
				if(GameTitle >= 0 && GameTrained == 0 && GameHasCheats == 1 && trainer == 1 && i <= daTrack_ptr) GameTrainer(outbuf);
				if(GameTitle >= 0 && GameFixed == 0 && fix_game == 1 && i <= daTrack_ptr) GameFixer(outbuf);
				if(vmode == 1 && i <= daTrack_ptr) NTSCpatcher(outbuf, i);
				if(i + headersize >= bin_size) {
					fwrite(outbuf, headersize - (i + headersize - bin_size), 1, file);
				} else fwrite(outbuf, headersize, 1, file);
			}
		}
		if(GameTitle >= 0 && fix_game == 1 && GameFixed == 0) {
			printf("COULD NOT APPLY THE GAME FIXE(S) : No data to patch found\n");
			printf("----------------------------------------------------------------------------------\n");
		}
		if(GameTitle >= 0 && GameHasCheats == 1 && GameTrained == 0 && trainer == 1) {
			printf("COULD NOT APPLY THE GAME CHEAT(S) : No data to patch found\n");
			printf("----------------------------------------------------------------------------------\n");
		}
		fclose(bin_file);
		fclose(file);

		printf("A POPS virtual CD-ROM image was saved to :\n");
		printf("%s\n\n", argv[2]);
		return 1;
	}

	/* 3 user arguments : 1 command, the command is user argument 2, output file is user argument 3 */
	if(((gap_more == 0 && vmode == 0 && trainer == 1) || (gap_less == 0 && vmode == 0 && trainer == 1) || (gap_more == 1 && vmode == 0 && trainer == 0) || (gap_less == 1 && vmode == 0 && trainer == 0) || (gap_more == 0 && gap_less == 0 && vmode == 1 && trainer == 0)) && argc == 4) {
		printf("Saving the virtual CD-ROM image. Please wait...\n");
		if(!(file = fopen(argv[3], "wb"))) {
			printf("Error : Cannot write to %s\n\n", argv[3]);
			free(bin_path);
			free(headerbuf);
			return 0;
		}
		fwrite(headerbuf, 1, headersize, file);
		fclose(file);
		free(headerbuf);


		if(!(file = fopen(argv[3], "ab+"))) {
			printf("Error : Cannot write to %s\n\n", argv[3]);
			free(bin_path);
			return 0;
		}

		if(!(bin_file = fopen(bin_path, "rb"))) {
			printf("Error: Cannot open %s\n\n", bin_path);
			free(bin_path);
			return 0;
		}
		free(bin_path);

		for(i = 0; i < bin_size; i += headersize) {
			if(fix_CDRWIN == 1 && (i + headersize >= daTrack_ptr)) {
				if(debug != 0) printf("Padding the CDRWIN dump inside of the virtual CD-ROM image...");
				fread(outbuf, headersize - (i + headersize - daTrack_ptr), 1, bin_file);
				fwrite(outbuf, headersize - (i + headersize - daTrack_ptr), 1, file);
				char padding[(150 * sectorsize) * 2];
				fwrite(padding, 150 * sectorsize, 1, file);
				fread(outbuf, headersize - (headersize - (i + headersize - daTrack_ptr)), 1, bin_file);
				fwrite(outbuf, headersize - (headersize - (i + headersize - daTrack_ptr)), 1, file);
				fix_CDRWIN = 0;
				if(debug != 0) {
					printf(" Done.\n");
					printf("Continuing...\n");
				}
				//if(vmode == 1) printf("----------------------------------------------------------------------------------\n");
			} else {
				if(vmode == 1 && i == 0) {
					printf("----------------------------------------------------------------------------------\n");
					printf("NTSC Patcher is ON\n");
					printf("----------------------------------------------------------------------------------\n");
				}
				fread(outbuf, headersize, 1, bin_file);
				if(i == 0) GameIdentifier(outbuf);
				if(GameTitle >= 0 && GameHasCheats == 1 && trainer == 1 && i == 0) {
					printf("GameTrainer is ON\n");
					printf("----------------------------------------------------------------------------------\n");
				}
				if(GameTitle >= 0 && GameTrained == 0 && GameHasCheats == 1 && trainer == 1 && i <= daTrack_ptr) GameTrainer(outbuf);
				if(GameTitle >= 0 && GameFixed == 0 && fix_game == 1 && i <= daTrack_ptr) GameFixer(outbuf);
				if(vmode == 1 && i <= daTrack_ptr) NTSCpatcher(outbuf, i);
				if(i + headersize >= bin_size) {
					fwrite(outbuf, headersize - (i + headersize - bin_size), 1, file);
				} else fwrite(outbuf, headersize, 1, file);
			}
		}
		if(GameTitle >= 0 && fix_game == 1 && GameFixed == 0) {
			printf("COULD NOT APPLY THE GAME FIXE(S) : No data to patch found\n");
			printf("----------------------------------------------------------------------------------\n");
		}
		if(GameTitle >= 0 && GameHasCheats == 1 && GameTrained == 0 && trainer == 1) {
			printf("COULD NOT APPLY THE GAME CHEAT(S) : No data to patch found\n");
			printf("----------------------------------------------------------------------------------\n");
		}
		fclose(bin_file);
		fclose(file);

		printf("A POPS virtual CD-ROM image was saved to :\n");
		printf("%s\n\n", argv[3]);
		return 1;
	}

	/* 4 user arguments : 2 commands, commands are user arguments 2 and 3, output file is user argument 4 */
	if(((gap_more == 1 && vmode == 0 && trainer == 1) || (gap_less == 1 && vmode == 0 && trainer == 1) || (gap_more == 1 && vmode == 1 && trainer == 0) || (gap_less == 1 && vmode == 1 && trainer == 0) || (gap_more == 0 && gap_less == 0 && vmode == 1 && trainer == 1)) && argc == 5) {
		printf("Saving the virtual CD-ROM image. Please wait...\n");
		if(!(file = fopen(argv[4], "wb"))) {
			printf("Error : Cannot write to %s\n\n", argv[4]);
			free(bin_path);
			free(headerbuf);
			return 0;
		}
		fwrite(headerbuf, 1, headersize, file);
		fclose(file);
		free(headerbuf);


		if(!(file = fopen(argv[4], "ab+"))) {
			printf("Error : Cannot write to %s\n\n", argv[4]);
			free(bin_path);
			return 0;
		}

		if(!(bin_file = fopen(bin_path, "rb"))) {
			printf("Error: Cannot open %s\n\n", bin_path);
			free(bin_path);
			return 0;
		}
		free(bin_path);

		for(i = 0; i < bin_size; i += headersize) {
			if(fix_CDRWIN == 1 && (i + headersize >= daTrack_ptr)) {
				if(debug != 0) printf("Padding the CDRWIN dump inside of the virtual CD-ROM image...");
				fread(outbuf, headersize - (i + headersize - daTrack_ptr), 1, bin_file);
				fwrite(outbuf, headersize - (i + headersize - daTrack_ptr), 1, file);
				char padding[(150 * sectorsize) * 2];
				fwrite(padding, 150 * sectorsize, 1, file);
				fread(outbuf, headersize - (headersize - (i + headersize - daTrack_ptr)), 1, bin_file);
				fwrite(outbuf, headersize - (headersize - (i + headersize - daTrack_ptr)), 1, file);
				fix_CDRWIN = 0;
				if(debug != 0) {
					printf(" Done.\n");
					printf("Continuing...\n");
				}
				//if(vmode == 1) printf("----------------------------------------------------------------------------------\n");
			} else {
				if(vmode == 1 && i == 0) {
					printf("----------------------------------------------------------------------------------\n");
					printf("NTSC Patcher is ON\n");
					printf("----------------------------------------------------------------------------------\n");
				}
				fread(outbuf, headersize, 1, bin_file);
				if(i == 0) GameIdentifier(outbuf);
				if(GameTitle >= 0 && GameHasCheats == 1 && trainer == 1 && i == 0) {
					printf("GameTrainer is ON\n");
					printf("----------------------------------------------------------------------------------\n");
				}
				if(GameTitle >= 0 && GameTrained == 0 && GameHasCheats == 1 && trainer == 1 && i <= daTrack_ptr) GameTrainer(outbuf);
				if(GameTitle >= 0 && GameFixed == 0 && fix_game == 1 && i <= daTrack_ptr) GameFixer(outbuf);
				if(vmode == 1 && i <= daTrack_ptr) NTSCpatcher(outbuf, i);
				if(i + headersize >= bin_size) {
					fwrite(outbuf, headersize - (i + headersize - bin_size), 1, file);
				} else fwrite(outbuf, headersize, 1, file);
			}
		}
		if(GameTitle >= 0 && fix_game == 1 && GameFixed == 0) {
			printf("COULD NOT APPLY THE GAME FIXE(S) : No data to patch found\n");
			printf("----------------------------------------------------------------------------------\n");
		}
		if(GameTitle >= 0 && GameHasCheats == 1 && GameTrained == 0 && trainer == 1) {
			printf("COULD NOT APPLY THE GAME CHEAT(S) : No data to patch found\n");
			printf("----------------------------------------------------------------------------------\n");
		}
		fclose(bin_file);
		fclose(file);

		printf("A POPS virtual CD-ROM image was saved to :\n");
		printf("%s\n\n", argv[4]);
		return 1;
	}

	/* 5 user arguments : 3 commands; Commands are user arguments 2, 3 and 4; Output file is user argument 5 */
	if(argc == 6) {
		printf("Saving the virtual CD-ROM image. Please wait...\n");
		if(!(file = fopen(argv[5], "wb"))) {
			printf("Error : Cannot write to %s\n\n", argv[5]);
			free(bin_path);
			free(headerbuf);
			return 0;
		}
		fwrite(headerbuf, 1, headersize, file);
		fclose(file);
		free(headerbuf);


		if(!(file = fopen(argv[5], "ab+"))) {
			printf("Error : Cannot write to %s\n\n", argv[5]);
			free(bin_path);
			return 0;
		}

		if(!(bin_file = fopen(bin_path, "rb"))) {
			printf("Error: Cannot open %s\n\n", bin_path);
			free(bin_path);
			return 0;
		}
		free(bin_path);

		for(i = 0; i < bin_size; i += headersize) {
			if(fix_CDRWIN == 1 && (i + headersize >= daTrack_ptr)) {
				if(debug != 0) printf("Padding the CDRWIN dump inside of the virtual CD-ROM image...");
				fread(outbuf, headersize - (i + headersize - daTrack_ptr), 1, bin_file);
				fwrite(outbuf, headersize - (i + headersize - daTrack_ptr), 1, file);
				char padding[(150 * sectorsize) * 2];
				fwrite(padding, 150 * sectorsize, 1, file);
				fread(outbuf, headersize - (headersize - (i + headersize - daTrack_ptr)), 1, bin_file);
				fwrite(outbuf, headersize - (headersize - (i + headersize - daTrack_ptr)), 1, file);
				fix_CDRWIN = 0;
				if(debug != 0) {
					printf(" Done.\n");
					printf("Continuing...\n");
				}
				//if(vmode == 1) printf("----------------------------------------------------------------------------------\n");
			} else {
				if(vmode == 1 && i == 0) {
					printf("----------------------------------------------------------------------------------\n");
					printf("NTSC Patcher is ON\n");
					printf("----------------------------------------------------------------------------------\n");
				}
				fread(outbuf, headersize, 1, bin_file);
				if(i == 0) GameIdentifier(outbuf);
				if(GameTitle >= 0 && GameHasCheats == 1 && trainer == 1 && i == 0) {
					printf("GameTrainer is ON\n");
					printf("----------------------------------------------------------------------------------\n");
				}
				if(GameTitle >= 0 && GameTrained == 0 && GameHasCheats == 1 && trainer == 1 && i <= daTrack_ptr) GameTrainer(outbuf);
				if(GameTitle >= 0 && GameFixed == 0 && fix_game == 1 && i <= daTrack_ptr) GameFixer(outbuf);
				if(vmode == 1 && i <= daTrack_ptr) NTSCpatcher(outbuf, i);
				if(i + headersize >= bin_size) {
					fwrite(outbuf, headersize - (i + headersize - bin_size), 1, file);
				} else fwrite(outbuf, headersize, 1, file);
			}
		}
		if(GameTitle >= 0 && fix_game == 1 && GameFixed == 0) {
			printf("COULD NOT APPLY THE GAME FIXE(S) : No data to patch found\n");
			printf("----------------------------------------------------------------------------------\n");
		}
		if(GameTitle >= 0 && GameHasCheats == 1 && GameTrained == 0 && trainer == 1) {
			printf("COULD NOT APPLY THE GAME CHEAT(S) : No data to patch found\n");
			printf("----------------------------------------------------------------------------------\n");
		}
		fclose(bin_file);
		fclose(file);

		printf("A POPS virtual CD-ROM image was saved to :\n");
		printf("%s\n\n", argv[5]);
		return 1;
	}


	/* Default, if none of the above cases returned or argc == 2, output file is the input file name plus the extension ".VCD" */
	i = strlen(argv[1]);
	for(; i > 0; i--) { // Search the extension ".cue" and ".CUE" in the input file name
		if((argv[1][i] == '.' && argv[1][i+1] == 'c' && argv[1][i+2] == 'u' && argv[1][i+3] == 'e') || (argv[1][i] == '.' && argv[1][i+1] == 'C' && argv[1][i+2] == 'U' && argv[1][i+3] == 'E'))
		break;
	}
	if(i >= 0) { // Found extension ".cue" or ".CUE", replace it with ".VCD"
		if(argv[1][i] == '"') argv[1][i + 4] = '"';
		else argv[1][i + 4] = '\0';
		argv[1][i + 1] = 'V';
		argv[1][i + 2] = 'C';
		argv[1][i + 3] = 'D';
	} else { // Extension ".cue" or ".CUE" not found, put ".VCD" at the end of the input file name
		i = strlen(argv[1]);
		if(argv[1][i] == '"') argv[1][i + 4] = '"';
		else argv[1][i + 4] = '\0';
		argv[1][i] = '.';
		argv[1][i + 1] = 'V';
		argv[1][i + 2] = 'C';
		argv[1][i + 3] = 'D';
	}

	printf("Saving the virtual CD-ROM image. Please wait...\n");
	if(!(file = fopen(argv[1], "wb"))) {
		printf("Error : Cannot write to %s\n\n", argv[1]);
		free(bin_path);
		free(headerbuf);
		return 0;
	}
	fwrite(headerbuf, 1, headersize, file);
	fclose(file);
	free(headerbuf);


	if(!(file = fopen(argv[1], "ab+"))) {
		printf("Error : Cannot write to %s\n\n", argv[1]);
		free(bin_path);
		return 0;
	}

	if(!(bin_file = fopen(bin_path, "rb"))) {
		printf("Error: Cannot open %s\n\n", bin_path);
		free(bin_path);
		return 0;
	}
	free(bin_path);

	for(i = 0; i < bin_size; i += headersize) {
		if(fix_CDRWIN == 1 && (i + headersize >= daTrack_ptr)) {
			if(debug != 0) printf("Padding the CDRWIN dump inside of the virtual CD-ROM image...");
			fread(outbuf, headersize - (i + headersize - daTrack_ptr), 1, bin_file);
			fwrite(outbuf, headersize - (i + headersize - daTrack_ptr), 1, file);
			char padding[(150 * sectorsize) * 2];
			fwrite(padding, 150 * sectorsize, 1, file);
			fread(outbuf, headersize - (headersize - (i + headersize - daTrack_ptr)), 1, bin_file);
			fwrite(outbuf, headersize - (headersize - (i + headersize - daTrack_ptr)), 1, file);
			fix_CDRWIN = 0;
			if(debug != 0) {
				printf(" Done.\n");
				printf("Continuing...\n");
			}
			//if(vmode == 1) printf("----------------------------------------------------------------------------------\n");
		} else {
			if(vmode == 1 && i == 0) {
				printf("----------------------------------------------------------------------------------\n");
				printf("NTSC Patcher is ON\n");
				printf("----------------------------------------------------------------------------------\n");
			}
			fread(outbuf, headersize, 1, bin_file);
			if(i == 0) GameIdentifier(outbuf);
			if(GameTitle >= 0 && GameHasCheats == 1 && trainer == 1 && i == 0) {
				printf("GameTrainer is ON\n");
				printf("----------------------------------------------------------------------------------\n");
			}
			if(GameTitle >= 0 && GameTrained == 0 && GameHasCheats == 1 && trainer == 1 && i <= daTrack_ptr) GameTrainer(outbuf);
			if(GameTitle >= 0 && GameFixed == 0 && fix_game == 1 && i <= daTrack_ptr) GameFixer(outbuf);
			if(vmode == 1 && i <= daTrack_ptr) NTSCpatcher(outbuf, i);
			if(i + headersize >= bin_size) {
				fwrite(outbuf, headersize - (i + headersize - bin_size), 1, file);
			} else fwrite(outbuf, headersize, 1, file);
		}
	}
	if(GameTitle >= 0 && fix_game == 1 && GameFixed == 0) {
		printf("COULD NOT APPLY THE GAME FIXE(S) : No data to patch found\n");
		printf("----------------------------------------------------------------------------------\n");
	}
	if(GameTitle >= 0 && GameHasCheats == 1 && GameTrained == 0 && trainer == 1) {
		printf("COULD NOT APPLY THE GAME CHEAT(S) : No data to patch found\n");
		printf("----------------------------------------------------------------------------------\n");
	}
	fclose(bin_file);
	fclose(file);

	printf("A POPS virtual CD-ROM image was saved to :\n");
	printf("%s\n\n", argv[1]);

	return 1;
}
/* EOSRC, oh mah dayum */
