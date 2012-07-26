/*
 * This confidential and proprietary software may be used only as
 * authorised by a licensing agreement from ARM Limited
 * (C) COPYRIGHT 2010-2011 ARM Limited
 * ALL RIGHTS RESERVED
 * The entire notice above must be reproduced on all authorised
 * copies and copies may only be made to the extent permitted
 * by a licensing agreement from ARM Limited.
 */

#ifndef _CINSTR_PROFILING_EVENTS_M200_H_
#define _CINSTR_PROFILING_EVENTS_M200_H_

/*
 * The event ID is a 32 bit value consisting of different fields
 * reserved, 4 bits, for future use
 * event type, 4 bits, cinstr_profiling_event_type_t
 * event channel, 8 bits, the source of the event.
 * event data, 16 bit field, data depending on event type
 */

/**
 * Specifies what kind of event this is
 */
typedef enum
{
	MALI_PROFILING_EVENT_TYPE_SINGLE  = 0 << 24,
	MALI_PROFILING_EVENT_TYPE_START   = 1 << 24,
	MALI_PROFILING_EVENT_TYPE_STOP    = 2 << 24,
	MALI_PROFILING_EVENT_TYPE_SUSPEND = 3 << 24,
	MALI_PROFILING_EVENT_TYPE_RESUME  = 4 << 24,
} cinstr_profiling_event_type_t;


/**
 * Secifies the channel/source of the event
 */
typedef enum
{
	MALI_PROFILING_EVENT_CHANNEL_SOFTWARE = 0 << 16,
	MALI_PROFILING_EVENT_CHANNEL_GP0      = 1 << 16,
	MALI_PROFILING_EVENT_CHANNEL_PP0      = 5 << 16,
	MALI_PROFILING_EVENT_CHANNEL_PP1      = 6 << 16,
	MALI_PROFILING_EVENT_CHANNEL_PP2      = 7 << 16,
	MALI_PROFILING_EVENT_CHANNEL_PP3      = 8 << 16,
} cinstr_profiling_event_channel_t;


#define MALI_PROFILING_MAKE_EVENT_CHANNEL_GP(num) (((MALI_PROFILING_EVENT_CHANNEL_GP0 >> 16) + (num)) << 16)
#define MALI_PROFILING_MAKE_EVENT_CHANNEL_PP(num) (((MALI_PROFILING_EVENT_CHANNEL_PP0 >> 16) + (num)) << 16)

/**
 * These events are applicable when the type MALI_PROFILING_EVENT_TYPE_SINGLE is used from software channel
 */
typedef enum
{
	MALI_PROFILING_EVENT_REASON_SINGLE_SW_NONE          = 0,
	MALI_PROFILING_EVENT_REASON_SINGLE_SW_EGL_NEW_FRAME = 1,
	MALI_PROFILING_EVENT_REASON_SINGLE_SW_FLUSH			= 2,
} cinstr_profiling_event_reason_single_sw_t;

/**
 * These events are applicable when the type MALI_PROFILING_EVENT_TYPE_START/STOP is used from software channel
 */
typedef enum
{
	MALI_PROFILING_EVENT_REASON_START_STOP_SW_NONE      = 0,
	MALI_PROFILING_EVENT_REASON_START_STOP_MALI         = 1,
} cinstr_profiling_event_reason_start_stop_sw_t;

/**
 * These events are applicable when the type MALI_PROFILING_EVENT_TYPE_SUSPEND/RESUME is used from software channel
 */
typedef enum
{
	MALI_PROFILING_EVENT_REASON_SUSPEND_RESUME_SW_NONE          = 0,
	MALI_PROFILING_EVENT_REASON_SUSPEND_RESUME_SW_PIPELINE_FULL = 1,
} cinstr_profiling_event_reason_suspend_resume_sw_t;

/**
 * These events are applicable when the type MALI_PROFILING_EVENT_TYPE_SINGLE is used from a HW channel (GPx+PPx)
 */
typedef enum
{
	MALI_PROFILING_EVENT_REASON_SINGLE_HW_NONE          = 0,
	MALI_PROFILING_EVENT_REASON_SINGLE_HW_INTERRUPT     = 1,
	MALI_PROFILING_EVENT_REASON_SINGLE_HW_FLUSH         = 2,
} cinstr_profiling_event_reason_single_hw_t;


#endif /*_CINSTR_PROFILING_EVENTS_M200_H_*/
