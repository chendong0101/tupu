// vim:set ts=4 sw=4 et:
#pragma once

#include "log.h"
#include "LogUtil.h"
#include <sys/types.h>
#include <stdint.h>

namespace platform {

class Log_Module;

/// Defines the structure of an ACE logging record.
class Log_Record
{
public:

    Log_Record (void);
    Log_Record (Log_Priority lp,
            time_t time_stamp,
            int tid);
    Log_Record (Log_Priority lp,
            const struct timeval& tv_stamp,
            int tid);

    /// Default dtor.
    ~Log_Record (void);

    /// disallow copying...
    Log_Record (const Log_Record& rhs) = delete;
    Log_Record& operator= (const Log_Record& rhs) = delete;

    /// Moving ctor.
    Log_Record (Log_Record&& rhs);
    Log_Record& operator= (Log_Record&& rhs);

    /// Get the type of the Log_Record.
    Log_Priority type (void) const;

    /// Set the type of the Log_Record.
    void type (Log_Priority);

    /// Get the module/category of the Log_Record.
    const Log_Module* module (void) const;
    /// Set the module/category of the Log_Record.
    void module (const Log_Module*);

    ///**
    // * Get the priority of the Log_Record <type_>.  This is computed
    // * as the base 2 logarithm of <type_> (which must be a power of 2,
    // * as defined by the enums in Log_Priority).
    // */
    //u_long priority (void) const;

    ///// Set the priority of the Log_Record <type_> (which must be a
    ///// power of 2, as defined by the enums in Log_Priority).
    //void priority (u_long num);

    ///// Get the total length of the Log_Record, which includes the
    ///// size of the various data member fields.
    //long length (void) const;

    ///// Set the total length of the Log_Record, which needs to account for
    ///// the size of the various data member fields.
    //void length (long);

    /// Get the time stamp of the Log_Record.
    const struct timeval& time_stamp (void) const;

    /// Set the time stamp of the Log_Record.
    void time_stamp (const struct timeval& tv);

    /// Get the task/thread id of the Log_Record.
    int tid (void) const;

    /// Set the task/thread id of the Log_Record.
    void tid (int);

    /// Get the message data of the Log_Record.
    const char *msg_data (void) const;

    /// Set the message data of the record. If @a data is longer than the
    /// current msg_data_ buffer, a new msg_data_ buffer is allocated to
    /// fit. If such a reallocation faisl, this method returns -1, else 0.
    //int msg_data (const char *data);

    /// Set the message data of the record. Give the buffer to Log_Record.
    /// Log_Record will free data by itself.
    int msg_data (char *data, int len, size_t size);

    /// Get the size of the message data of the Log_Record, including
    /// a byte for the NUL.
    size_t msg_data_len (void) const;


private:

    ///// Round up to the alignment restrictions.
    //void round_up (void);

    ///**
    // * Total length of the logging record in bytes.  This field *must*
    // * come first in order for various IPC framing mechanisms to work
    // * correctly.  In addition, the field must be an ACE_INT32 in order
    // * to be passed portably across platforms.
    // */
    //uint32_t length_;

    /// Type of logging record.
    Log_Priority type_;

    /// Time that the logging record was generated.
    struct timeval tv_;

    /// Id of task/thread that generated the logging record.
    uint32_t tid_;

    /// Logging record data
    char *msg_data_;   // Heap-allocated text message area

    /// Data size of msg_data_ in ACE_TCHARs
    size_t msg_data_len_;   // 

    /// Allocated size of msg_data_ in ACE_TCHARs
    size_t msg_data_size_;

    /// log module or category
    const Log_Module* module_;
};

} //namespace

