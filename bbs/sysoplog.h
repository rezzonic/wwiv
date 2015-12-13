/**************************************************************************/
/*                                                                        */
/*                              WWIV Version 5.x                          */
/*             Copyright (C)1998-2015, WWIV Software Services             */
/*                                                                        */
/*    Licensed  under the  Apache License, Version  2.0 (the "License");  */
/*    you may not use this  file  except in compliance with the License.  */
/*    You may obtain a copy of the License at                             */
/*                                                                        */
/*                http://www.apache.org/licenses/LICENSE-2.0              */
/*                                                                        */
/*    Unless  required  by  applicable  law  or agreed to  in  writing,   */
/*    software  distributed  under  the  License  is  distributed on an   */
/*    "AS IS"  BASIS, WITHOUT  WARRANTIES  OR  CONDITIONS OF ANY  KIND,   */
/*    either  express  or implied.  See  the  License for  the specific   */
/*    language governing permissions and limitations under the License.   */
/**************************************************************************/
#ifndef __INCLUDED_SYSOPLOG_H__
#define __INCLUDED_SYSOPLOG_H__

#include <string>

std::string GetSysopLogFileName(const std::string& date);
void GetTemporaryInstanceLogFileName(char *pszInstanceLogFileName);
void catsl();
void sysopchar(const std::string& text);
void sysoplog(const std::string& text, bool bIndent = true);
void sysoplogf(const char *format, ...);
void sysoplogfi(bool bIndent, const char *format, ...);



#endif  // __INCLUDED_SYSOPLOG_H__