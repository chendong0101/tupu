// vim:set ts=4 sw=4 et:
/**
 *  \file log utilities
 */

#define __STDC_FORMAT_MACROS 1
#include "LogRecord.h"
#include "LogImpl.h"
#include "log.h"

#include <string>
#include <cstring>
#include <cstdarg>

static inline u_long
log2 (u_long num)
{
      u_long log = 0;

        for (; num > 1; ++log)
                num >>= 1;

          return log;
}

namespace platform {

Log_Record::Log_Record (void)
    : type_ (Log_Priority(0)),
    tv_ ({0,0}),
    tid_ (0),
    msg_data_ (0),
    msg_data_size_ (0),
    module_(0)
{
}

Log_Record::Log_Record (Log_Record && rhs)
    : type_ (rhs.type_),
    tv_ (rhs.tv_),
    tid_ (rhs.tid_),
    msg_data_ (rhs.msg_data_),
    msg_data_len_ (rhs.msg_data_len_),
    msg_data_size_ (rhs.msg_data_size_),
    module_ (rhs.module_)
{
    rhs.msg_data_ = NULL;
    rhs.msg_data_len_ = 0;
    rhs.msg_data_size_ = 0;
    rhs.module_ = NULL;
}

Log_Record::Log_Record (Log_Priority lp,
                        time_t ts_sec,
                        int tid)
    : type_ (lp),
    tv_ ({ts_sec,0}),
    tid_ (uint32_t (tid)),
    msg_data_ (0),
    msg_data_len_ (0),
    msg_data_size_ (0),
    module_ (0)
{
}

Log_Record::Log_Record (Log_Priority lp,
                        const struct timeval &tv,
                        int tid)
    : type_ (lp),
    tv_ (tv),
    tid_ (uint32_t (tid)),
    msg_data_ (0),
    msg_data_len_ (0),
    msg_data_size_ (0),
    module_ (0)
{
}

Log_Record::~Log_Record (void)
{
    if (this->msg_data_ && this->msg_data_size_)
        free(this->msg_data_);
}

Log_Record& Log_Record::operator= (Log_Record && rhs)
{
    if (this->msg_data_ && this->msg_data_size_)
        free(this->msg_data_);

    //this->length_ = rhs.length_;
    this->type_ = rhs.type_;
    this->tv_ = rhs.tv_;
    this->tid_ = rhs.tid_;
    this->msg_data_ = rhs.msg_data_;
    this->msg_data_len_ = rhs.msg_data_len_;
    this->msg_data_size_ = rhs.msg_data_size_;
    this->module_ = rhs.module_;

    //rhs.length_ = 0;
    rhs.msg_data_ = NULL;
    rhs.msg_data_len_ = 0;
    rhs.msg_data_size_ = 0;
    rhs.module_ = NULL;

    return *this;
}

int Log_Record::msg_data (char *data, int len, size_t size)
{
    // //TRACE ("Log_Record::msg_data");
    if (this->msg_data_ && this->msg_data_size_)
        free(this->msg_data_);

    this->msg_data_ = data;
    this->msg_data_len_ = len;
    this->msg_data_size_ = size;

    return 0;
}


Log_Priority
Log_Record::type (void) const
{
  //TRACE ("Log_Record::type");
  return this->type_;
}

void
Log_Record::module (const Log_Module* t)
{
  //TRACE ("Log_Record::module");
  this->module_ = t;
}


const Log_Module*
Log_Record::module (void) const
{
  //TRACE ("Log_Record::module");
  return this->module_;
}

void
Log_Record::type (Log_Priority t)
{
  //TRACE ("Log_Record::type");
  this->type_ = t;
}

const struct timeval&
Log_Record::time_stamp (void) const
{
  //TRACE ("Log_Record::time_stamp");
  return this->tv_;
}

void
Log_Record::time_stamp (const struct timeval &tv)
{
  //TRACE ("Log_Record::time_stamp");
  this->tv_ = tv;
}

int
Log_Record::tid (void) const
{
  //TRACE ("Log_Record::tid");
  return (long) this->tid_;
}

void
Log_Record::tid (int p)
{
  //TRACE ("Log_Record::tid");
  this->tid_ = p;
}

const char *
Log_Record::msg_data (void) const
{
  //TRACE ("Log_Record::msg_data");
  return this->msg_data_;
}

size_t
Log_Record::msg_data_len (void) const
{
  //TRACE ("Log_Record::msg_data_len");
  //return strlen (this->msg_data_) + 1;
  return this->msg_data_len_;
}


} // namespace
