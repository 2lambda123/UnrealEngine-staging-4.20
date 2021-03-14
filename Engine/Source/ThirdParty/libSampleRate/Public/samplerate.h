#pragma once
/*
** Copyright (c) 2002-2016, Erik de Castro Lopo <erikd@mega-nerd.com>
** All rights reserved.
**
** This code is released under 2-clause BSD license. Please see the
** file at : https://github.com/erikd/libsamplerate/blob/master/COPYING
*/

/*
** API documentation is available here:
**     http://www.mega-nerd.com/SRC/api.html
*/

#ifdef PLATFORM_WINDOWS
	#if PLATFORM_WINDOWS
	#define SRC_EXPORT __declspec(dllexport)
	#else
	#define SRC_EXPORT
	#endif
#else
	#define SRC_EXPORT
#endif


#ifdef __cplusplus
extern "C" {
#endif	/* __cplusplus */


/* Opaque data type SRC_STATE. */
typedef struct SRC_STATE_tag SRC_STATE ;

/* SRC_DATA is used to pass data to src_simple() and src_process(). */
typedef struct
{	const float	*data_in ;
	float	 *data_out ;

	long	input_frames, output_frames ;
	long	input_frames_used, output_frames_gen ;

	int		end_of_input ;

	double	src_ratio ;
} SRC_DATA ;

/*
** User supplied callback function type for use with src_callback_new()
** and src_callback_read(). First parameter is the same pointer that was
** passed into src_callback_new(). Second parameter is pointer to a
** pointer. The user supplied callback function must modify *data to
** point to the start of the user supplied float array. The user supplied
** function must return the number of frames that **data points to.
*/

typedef long (*src_callback_t) (void *cb_data, float **data) ;

/*
**	Standard initialisation function : return an anonymous pointer to the
**	internal state of the converter. Choose a converter from the enums below.
**	Error returned in *error.
*/

SRC_EXPORT SRC_STATE* src_new (int converter_type, int channels, int *error) ;

/*
**	Initilisation for callback based API : return an anonymous pointer to the
**	internal state of the converter. Choose a converter from the enums below.
**	The cb_data pointer can point to any data or be set to NULL. Whatever the
**	value, when processing, user supplied function "func" gets called with
**	cb_data as first parameter.
*/

SRC_EXPORT SRC_STATE* src_callback_new (src_callback_t func, int converter_type, int channels,
				int *error, void* cb_data) ;

/*
**	Cleanup all internal allocations.
**	Always returns NULL.
*/

SRC_EXPORT SRC_STATE* src_delete (SRC_STATE *state) ;

/*
**	Standard processing function.
**	Returns non zero on error.
*/

SRC_EXPORT int src_process (SRC_STATE *state, SRC_DATA *data) ;

/*
**	Callback based processing function. Read up to frames worth of data from
**	the converter int *data and return frames read or -1 on error.
*/
SRC_EXPORT long src_callback_read (SRC_STATE *state, double src_ratio, long frames, float *data) ;

/*
**	Simple interface for performing a single conversion from input buffer to
**	output buffer at a fixed conversion ratio.
**	Simple interface does not require initialisation as it can only operate on
**	a single buffer worth of audio.
*/

SRC_EXPORT int src_simple (SRC_DATA *data, int converter_type, int channels) ;

/*
** This library contains a number of different sample rate converters,
** numbered 0 through N.
**
** Return a string giving either a name or a more full description of each
** sample rate converter or NULL if no sample rate converter exists for
** the given value. The converters are sequentially numbered from 0 to N.
*/

SRC_EXPORT const char *src_get_name (int converter_type) ;
SRC_EXPORT const char *src_get_description (int converter_type) ;
SRC_EXPORT const char *src_get_version (void) ;

/*
**	Set a new SRC ratio. This allows step responses
**	in the conversion ratio.
**	Returns non zero on error.
*/

SRC_EXPORT int src_set_ratio (SRC_STATE *state, double new_ratio) ;

/*
**	Get the current channel count.
**	Returns negative on error, positive channel count otherwise
*/

SRC_EXPORT int src_get_channels (SRC_STATE *state) ;

/*
**	Reset the internal SRC state.
**	Does not modify the quality settings.
**	Does not free any memory allocations.
**	Returns non zero on error.
*/

SRC_EXPORT int src_reset (SRC_STATE *state) ;

/*
** Return TRUE if ratio is a valid conversion ratio, FALSE
** otherwise.
*/

SRC_EXPORT int src_is_valid_ratio (double ratio) ;

/*
**	Return an error number.
*/

SRC_EXPORT int src_error (SRC_STATE *state) ;

/*
**	Convert the error number into a string.
*/
SRC_EXPORT const char* src_strerror (int error) ;

/*
** The following enums can be used to set the interpolator type
** using the function src_set_converter().
*/

enum
{
	SRC_SINC_BEST_QUALITY		= 0,
	SRC_SINC_MEDIUM_QUALITY		= 1,
	SRC_SINC_FASTEST			= 2,
	SRC_ZERO_ORDER_HOLD			= 3,
	SRC_LINEAR					= 4,
} ;

/*
** Extra helper functions for converting from short to float and
** back again.
*/

SRC_EXPORT void src_short_to_float_array (const short *in, float *out, int len) ;
SRC_EXPORT void src_float_to_short_array (const float *in, short *out, int len) ;

SRC_EXPORT void src_int_to_float_array (const int *in, float *out, int len) ;
SRC_EXPORT void src_float_to_int_array (const float *in, int *out, int len) ;


#ifdef __cplusplus
}		/* extern "C" */
#endif	/* __cplusplus */


