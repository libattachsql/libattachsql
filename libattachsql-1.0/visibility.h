/* vim:expandtab:shiftwidth=2:tabstop=2:smarttab:
 * Copyright 2014 Hewlett-Packard Development Company, L.P.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain 
 * a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 */
#if defined(ASQL_DLL)
# if defined(BUILDING_ASQL)
#  if defined(_MSC_VER)
#   define ASQL_API extern __declspec(dllexport)
#  elif defined(HAVE_VISIBILITY) && HAVE_VISIBILITY
#   define ASQL_API __attribute__ ((visibility("default")))
#  elif defined (__SUNPRO_C) && (__SUNPRO_C >= 0x550)
#   define ASQL_API __global
#  endif /* defined(HAVE_VISIBILITY) */
# else  /* defined(BUILDING_ASQL) */
#  if defined(_MSC_VER)
#   define ASQL_API extern __declspec(dllimport)
#  else
#   define ASQL_API
#  endif /* defined(_MSC_VER) */
# endif /* defined(BUILDING_ASQL) */
#endif /* defined(ASQL_DLL) */

#ifndef ASQL_API
# define ASQL_API
#endif