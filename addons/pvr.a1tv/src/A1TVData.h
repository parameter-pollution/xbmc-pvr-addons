#pragma once
/*
 *      Copyright (C) 2013 Anton Fedchin
 *      http://github.com/afedchin/xbmc-addon-iptvsimple/
 *
 *      Copyright (C) 2011 Pulse-Eight
 *      http://www.pulse-eight.com/
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include <vector>
#include <map>
#include <json/json.h>
#include "platform/util/StdString.h"
#include "client.h"
#include "platform/threads/threads.h"
#include "xbmc_pvr_types.h"
#include "xbmc_epg_types.h"

class A1TVData : public PLATFORM::CThread
{
public:
  A1TVData(void);
  virtual ~A1TVData(void);

  virtual int       GetChannelsAmount(void);
  virtual PVR_ERROR GetChannels(ADDON_HANDLE handle, bool bRadio);
  virtual bool      GetChannel(const PVR_CHANNEL &channel, PVR_CHANNEL &myChannel);
  virtual PVR_ERROR GetEPGForChannel(ADDON_HANDLE handle, const PVR_CHANNEL &channel, time_t iStart, time_t iEnd);
  virtual std::string GetChannelFile() const;

protected:
  virtual bool                 LoadChannelList(void);
  virtual int                  GetFileContents(std::string url, std::string &strContent);
  virtual bool                 GetJson(std::string url, Json::Value &response);
  virtual void                 DeepCopyChannel(PVR_CHANNEL &src, PVR_CHANNEL &dst);

  virtual time_t               ParseEPGDate(CStdString strDate);
  virtual int                  GetGenreType( std::string strGenre);
  virtual CStdString           ReadMarkerValue(std::string &strLine, const char * strMarkerName);
  virtual void *Process(void);

private:
  bool                              m_bTSOverride;
  bool                              m_bEGPLoaded;
  int                               m_iEPGTimeShift;
  int                               m_iLastStart;
  int                               m_iLastEnd;
  CStdString                        m_strM3uUrl;
  CStdString                        m_strLogoPath;
  std::vector<PVR_CHANNEL>       m_channels;
  std::vector<EPG_TAG>    m_epg;
  std::map<std::string, int> genre_map;
};
