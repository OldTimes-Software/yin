// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright © 2020-2022 Mark E Sowden <hogsy@oldtimes-software.com>

#pragma once

#include "engine_private.h"

PL_EXTERN_C

/**
 * This list provides a number of
 * somewhat standard presets for
 * reverb.
 */
typedef enum AudioReverbPreset
{
	AUDIO_REVERB_PRESET_FOREST,
	AUDIO_REVERB_PRESET_DEFAULT,
	AUDIO_REVERB_PRESET_GENERIC,
	AUDIO_REVERB_PRESET_PADDEDCELL,
	AUDIO_REVERB_PRESET_ROOM,
	AUDIO_REVERB_PRESET_BATHROOM,
	AUDIO_REVERB_PRESET_LIVINGROOM,
	AUDIO_REVERB_PRESET_STONEROOM,
	AUDIO_REVERB_PRESET_AUDITORIUM,
	AUDIO_REVERB_PRESET_CONCERTHALL,
	AUDIO_REVERB_PRESET_CAVE,
	AUDIO_REVERB_PRESET_ARENA,
	AUDIO_REVERB_PRESET_HANGAR,
	AUDIO_REVERB_PRESET_CARPETEDHALLWAY,
	AUDIO_REVERB_PRESET_HALLWAY,
	AUDIO_REVERB_PRESET_STONECORRIDOR,
	AUDIO_REVERB_PRESET_ALLEY,
	AUDIO_REVERB_PRESET_CITY,
	AUDIO_REVERB_PRESET_MOUNTAINS,
	AUDIO_REVERB_PRESET_QUARRY,
	AUDIO_REVERB_PRESET_PLAIN,
	AUDIO_REVERB_PRESET_PARKINGLOT,
	AUDIO_REVERB_PRESET_SEWERPIPE,
	AUDIO_REVERB_PRESET_UNDERWATER,
	AUDIO_REVERB_PRESET_SMALLROOM,
	AUDIO_REVERB_PRESET_MEDIUMROOM,
	AUDIO_REVERB_PRESET_LARGEROOM,
	AUDIO_REVERB_PRESET_MEDIUMHALL,
	AUDIO_REVERB_PRESET_LARGEHALL,
	AUDIO_REVERB_PRESET_PLATE,

	AUDIO_MAX_REVERB_PRESETS
} AudioReverbPreset;

/**
 * WARNING: DO NOT CHANGE THIS!!
 * This should match with the 'fmt ' structure
 * within a WAV file.
 */
typedef struct AudioWaveFormat
{
	uint16_t formatTag;
	uint16_t channels;
	uint32_t samplesPerSec;
	uint32_t avgBytesPerSec;
	uint16_t blockAlign;
	uint16_t bitsPerSample;
	uint16_t size;
} AudioWaveFormat;

typedef struct AudioSample
{
	char            path[ PL_SYSTEM_MAX_PATH ];
	bool            reserved;
	int             numReferences;
	int             channel;
	AudioWaveFormat format;
	uint8_t        *buffer;
	unsigned int    bufferSize;
	void           *user;
} AudioSample;

typedef struct AudioSource
{
	PLVector3 position;
	PLVector3 velocity;
	void     *user;
} AudioSource;

void Audio_Initialize( void );
void Audio_Shutdown( void );

void Audio_Tick( void );
void Audio_Pause( bool pause );

void      Audio_UpdateListener( const PLVector3 *position, const PLVector3 *angles, const PLVector3 *velocity );
void      Audio_ClearListener( void );
PLVector3 Audio_GetListenerPosition( void );
PLVector3 Audio_GetListenerAngles( void );
PLVector3 Audio_GetListenerVelocity( void );

float Audio_GetGlobalVolume( void );

void         Audio_CleanupSamples( bool force );
AudioSample *Audio_CacheSample( const char *path );
void         Audio_EmitSample( AudioSample *audioSample, int8_t volume );
void         Audio_ReleaseSample( AudioSample *audioSample );

AudioSource *Audio_Source_Create( const PLVector3 *position, const PLVector3 *velocity );
void         Audio_Source_Destroy( AudioSource *audioSource );
void         Audio_Source_Emit( AudioSource *audioSource, AudioSample *audioSample );

void *Audio_Wav_Load( const char *path, AudioWaveFormat *waveFormatEx, unsigned int *bufferSize );

typedef struct AudioDriverInterface
{
	bool ( *Initialize )( void );
	void ( *Shutdown )( void );
	void ( *Tick )( void );

	void ( *Pause )( bool pause );

	bool ( *CacheSample )( AudioSample *audioSample );
	void ( *FreeSample )( AudioSample *audioSample );
	void ( *EmitSample )( AudioSample *audioSample, int8_t volume );

	bool ( *CreateSource )( AudioSource *audioSource );
	void ( *DestroySource )( AudioSource *audioSource );
} AudioDriverInterface;

PL_EXTERN_C_END
