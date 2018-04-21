/******************************************************************************
 * 
 * Copyright 2017 karawin (http://www.karawin.fr)
 *
*******************************************************************************/

#pragma once

#include "esp_system.h"
#include "audio_renderer.h"

//define for bit array in theme
#define T_THEME 	1
#define NT_THEME	0xFE
#define T_PATCH 	2
#define NT_PATCH	0xFD
#define T_LED		4
#define NT_LED		0xFB
#define APMODE		0
#define STA1		1
#define STA2		2

struct device_settings {
	uint16_t cleared; 		// 0xAABB if initialized
	uint8_t dhcpEn1;
	uint8_t ipAddr1[4];
	uint8_t mask1[4];
	uint8_t gate1[4];
	uint8_t dhcpEn2;		
	uint8_t ipAddr2[4];		
	uint8_t mask2[4];		
	uint8_t gate2[4];			
	char ssid1[32]; 
	char ssid2[32]; 
	char pass1[64];
	char pass2[64];
	uint8_t current_ap; // 0 = AP mode, else STA mode: 1 = ssid1, 2 = ssid2
	uint8_t vol;
	int8_t treble;
	uint8_t bass;
	int8_t freqtreble;
	uint8_t freqbass;
	uint8_t spacial;
	uint16_t currentstation;  // 
	uint8_t autostart; // 0: stopped, 1: playing
	uint8_t i2sspeed; // 0 = 48kHz, 1 = 96kHz, 2 = 128kHz
	uint32_t uartspeed; // serial baud
	uint8_t options;  // bit0:0 theme ligth blue, 1 Dark brown, bit1: 0 patch load  1 no patch, bit2: O blink led  1 led on On play
	char ua[39]; // user agent
	int8_t tzoffset; //timezone offset
	uint32_t sleepValue; 	
	uint32_t wakeValue;	
// esp32
	output_mode_t audio_output_mode; // 
	uint8_t trace_level;
	uint8_t lcd_type; // the lcd in use
	uint8_t led_gpio; // the gpio of the led
	uint32_t lcd_out;	// timeout in seconds to switch off the lcd. 0 = no timeout
	uint8_t ddmm;		// 0 = MMDD, 1 = DDMM  in the time display
	char filler[32]; 

};

struct shoutcast_info {
	char domain[73]; //url
	char file[116];  //path
	char name[64];
	int8_t ovol; // offset volume
	uint16_t port;	//port
};

void partitions_init(void);
void copyDeviceSettings();
void restoreDeviceSettings();
bool eeSetData(int address, void* buffer, int size);
bool eeSetData1(int address, void* buffer, int size);
void eeErasesettings(void);
void eeEraseAll();
void saveStation(struct shoutcast_info *station, uint16_t position);
void saveMultiStation(struct shoutcast_info *station, uint16_t position, uint8_t number);
void eeEraseStations(void);
struct shoutcast_info* getStation(uint8_t position);
void saveDeviceSettings(struct device_settings *settings);
void saveDeviceSettingsVolume(struct device_settings *settings);
struct device_settings* getDeviceSettings();
struct device_settings* getDeviceSettingsSilent();
