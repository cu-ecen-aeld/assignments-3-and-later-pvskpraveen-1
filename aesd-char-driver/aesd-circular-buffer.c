/**
 * @file aesd-circular-buffer.c
 * @brief Functions and data related to a circular buffer imlementation
 *
 * @author Dan Walkes
 * @date 2020-03-01
 * @copyright Copyright (c) 2020
 *
 */

#ifdef __KERNEL__
#include <linux/string.h>
#else
#include <string.h>
#endif

#include "aesd-circular-buffer.h"

/**
 * @param buffer the buffer to search for corresponding offset.  Any necessary locking must be performed by caller.
 * @param char_offset the position to search for in the buffer list, describing the zero referenced
 *      character index if all buffer strings were concatenated end to end
 * @param entry_offset_byte_rtn is a pointer specifying a location to store the byte of the returned aesd_buffer_entry
 *      buffptr member corresponding to char_offset.  This value is only set when a matching char_offset is found
 *      in aesd_buffer.
 * @return the struct aesd_buffer_entry structure representing the position described by char_offset, or
 * NULL if this position is not available in the buffer (not enough data is written).
 */
struct aesd_buffer_entry *aesd_circular_buffer_find_entry_offset_for_fpos(struct aesd_circular_buffer *buffer,
            size_t char_offset, size_t *entry_offset_byte_rtn )
{
    /**
    * TODO: implement per description
    */
	uint8_t i;	 // i is used to loop for AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED iterations
	uint8_t curloc; // curloc is used to reevaluate i to the location based on where the out_offs is currently located
	size_t total_size = 0;  // accumulator to find the offset as requested by the caller
	
	// loops for max AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED iterations
	for (i=0; i<AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;i++) {
		
		// relocate the current list position(curloc) based on the location of out_offs
		if ((buffer->out_offs + i) > AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED-1)	{
			curloc = buffer->out_offs + i - AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;
		}
		else
			curloc = buffer->out_offs + i;
		
		// add the size value of the current list position to total_size. stop after encountering total_size greater than the offset provided by caller
		total_size = total_size + (buffer->entry[curloc]).size;
		
		// return the structure located at curloc
		if (total_size > char_offset)	 {
			*entry_offset_byte_rtn = char_offset - (total_size - (buffer->entry[curloc]).size);
			return &(buffer->entry[curloc]);
		}
		
		
		// if in_offs encountered without a hit, then returns NULL
		if (!(buffer->full) && curloc == buffer->in_offs-1)	{
			return NULL;
		}
	}
	// if the entire buffer is traversed without a hit, the funtion returns NULL
	return NULL;
}

/**
* Adds entry @param add_entry to @param buffer in the location specified in buffer->in_offs.
* If the buffer was already full, overwrites the oldest entry and advances buffer->out_offs to the
* new start location.
* Any necessary locking must be handled by the caller
* Any memory referenced in @param add_entry must be allocated by and/or must have a lifetime managed by the caller.
*/
void aesd_circular_buffer_add_entry(struct aesd_circular_buffer *buffer, const struct aesd_buffer_entry *add_entry)
{
    /**
    * TODO: implement per description
    */
	// Add the entry to the current location of in_offs
	buffer->entry[buffer->in_offs] = *add_entry;
	
	// Update the locations of in_offs and out_offs after addition of the new entry
	// Implement cicular buffer by returning back to 0 after encountering max location
	if (buffer->in_offs == AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED-1)	
		buffer->in_offs=0;
	else
		buffer->in_offs++;
	
	// Increment out_offs only if buffer is full and addition is performed.
	if (buffer->full)	{
		if (buffer->out_offs < AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED)
			buffer->out_offs++;
		else
			buffer->out_offs = 0;
	}			
	
	// checking for buffer full only done at the end to prevent out_offs incrementing as soon as buffer is full
	if (buffer->in_offs == buffer->out_offs)	{
		buffer->full = true;
	}
}

/**
* Initializes the circular buffer described by @param buffer to an empty struct
*/
void aesd_circular_buffer_init(struct aesd_circular_buffer *buffer)
{
    memset(buffer,0,sizeof(struct aesd_circular_buffer));
}
