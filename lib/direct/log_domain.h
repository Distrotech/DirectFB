/*
   (c) Copyright 2001-2008  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __DIRECT__LOG_DOMAIN_H__
#define __DIRECT__LOG_DOMAIN_H__

#include <direct/compiler.h>

/**********************************************************************************************************************/

typedef enum {
     DIRECT_LOG_NONE     = 0x0,
     DIRECT_LOG_FATAL    = 0x1,
     DIRECT_LOG_ERROR    = 0x2,
     DIRECT_LOG_WARNING  = 0x3,
     DIRECT_LOG_NOTICE   = 0x4,
     DIRECT_LOG_INFO     = 0x5,
     DIRECT_LOG_VERBOSE  = 0x6,

     DIRECT_LOG_DEBUG_0  = DIRECT_LOG_VERBOSE,

     DIRECT_LOG_DEBUG_1  = 0x7,
     DIRECT_LOG_DEBUG_2  = 0x8,
     DIRECT_LOG_DEBUG_3  = 0x9,
     DIRECT_LOG_DEBUG_4  = 0xA,
     DIRECT_LOG_DEBUG_5  = 0xB,
     DIRECT_LOG_DEBUG_6  = 0xC,
     DIRECT_LOG_DEBUG_7  = 0xD,
     DIRECT_LOG_DEBUG_8  = 0xE,
     DIRECT_LOG_DEBUG_9  = 0xF,

     DIRECT_LOG_ALL      = 0x10,

     DIRECT_LOG_DEBUG    = DIRECT_LOG_DEBUG_8,    /* default debug level */

     _DIRECT_LOG_NUM_LEVELS
} DirectLogLevel;


typedef struct {
     DirectLogLevel  level;
     DirectLog      *log;
} DirectLogDomainConfig;

/**********************************************************************************************************************/

typedef struct {
     unsigned int             age;
     //bool                     registered;

     DirectLogDomainConfig    config;

     const char              *name;
     const char              *description;

     int                      name_len;
} DirectLogDomain;

/**********************************************************************************************************************/

#define D_LOG_DOMAIN( _identifier, _name, _description )                                                           \
     static DirectLogDomain _identifier __attribute__((unused)) = {                                                \
            .description = _description,                                                                           \
            .name        =  _name,                                                                                 \
            .name_len    = sizeof(_name) - 1                                                                       \
     };

/**********************************************************************************************************************/

void direct_log_domain_configure( const char                  *name,
                                  const DirectLogDomainConfig *config );


DirectResult direct_log_domain_vprintf( DirectLogDomain *domain,
                                        DirectLogLevel   level,
                                        const char      *format,
                                        va_list          ap )       D_FORMAT_VPRINTF(3);

DirectResult direct_log_domain_log( DirectLogDomain *domain,
                                    DirectLogLevel   level,
                                    const char      *func,
                                    const char      *file,
                                    int              line,
                                    const char      *format, ... )  D_FORMAT_PRINTF(6);

/**********************************************************************************************************************/

static inline void
direct_log_domain_config_level( const char     *name,
                                DirectLogLevel  level )
{
     DirectLogDomainConfig config = {0};

     config.level = level;

     direct_log_domain_configure( name, &config );
}

/**********************************************************************************************************************/

#define D_LOG( _Domain, _LEVEL, _msg... )                                                                          \
     do {                                                                                                          \
          direct_log_domain_log( &(_Domain), DIRECT_LOG_ ## _LEVEL, __FUNCTION__, __FILE__, __LINE__, _msg );      \
     } while (0)


#endif

