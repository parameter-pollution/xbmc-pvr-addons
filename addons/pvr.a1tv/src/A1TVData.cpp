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

#include <sstream>
#include <string>
#include <fstream>
#include <map>
#include <math.h>
#include <json/json.h>
#include "A1TVData.h"


#define SECONDS_IN_DAY          86400
#define EPG_URL ""

using namespace std;
using namespace ADDON;

A1TVData::A1TVData(void)
{
  m_iEPGTimeShift = g_iEPGTimeShift;
  m_bTSOverride   = g_bTSOverride;
  m_iLastStart    = 0;
  m_iLastEnd      = 0;

  m_bEGPLoaded = false;

  //C++98 doesn't support list initialization at compile time so whe have to do it here. ( C++0x does allow it)
  //TODO??: put this map into a configfile so it is easier to edit it
  genre_map["Comedy"] =         EPG_EVENT_CONTENTMASK_SHOW;
  genre_map["Dokumentation"] =  EPG_EVENT_CONTENTMASK_EDUCATIONALSCIENCE;
  genre_map["Drama"] =          EPG_EVENT_CONTENTMASK_MOVIEDRAMA;
  genre_map["Fussball"] =       EPG_EVENT_CONTENTMASK_SPORTS;
  genre_map["Info"] =           EPG_EVENT_CONTENTMASK_EDUCATIONALSCIENCE;
  genre_map["Kinder"] =         EPG_EVENT_CONTENTMASK_CHILDRENYOUTH;
  genre_map["Krimi"] =          EPG_EVENT_CONTENTMASK_MOVIEDRAMA;
  genre_map["Lifestyle"] =      EPG_EVENT_CONTENTMASK_LEISUREHOBBIES;
  genre_map["Magazin"] =        EPG_EVENT_CONTENTMASK_LEISUREHOBBIES;
  genre_map["News"] =           EPG_EVENT_CONTENTMASK_NEWSCURRENTAFFAIRS;
  genre_map["Serie"] =          EPG_EVENT_CONTENTMASK_SHOW;
  genre_map["Show"] =           EPG_EVENT_CONTENTMASK_SHOW;
  genre_map["Spielfilm"] =      EPG_EVENT_CONTENTMASK_MOVIEDRAMA;
  genre_map["Sport"] =          EPG_EVENT_CONTENTMASK_SPORTS;
  genre_map["Talk"] =           EPG_EVENT_CONTENTMASK_SOCIALPOLITICALECONOMICS;
  genre_map["Wetter"] =         EPG_EVENT_CONTENTMASK_NEWSCURRENTAFFAIRS;
  genre_map["Romantik"] =       EPG_EVENT_CONTENTMASK_MOVIEDRAMA;
  genre_map["Komödie"] =        EPG_EVENT_CONTENTMASK_MOVIEDRAMA;

  XBMC->QueueNotification(QUEUE_INFO, "A1TV initialized");
}

void *A1TVData::Process(void)
{
  return NULL;
}

A1TVData::~A1TVData(void)
{
  m_channels.clear();
  m_epg.clear();
}

bool A1TVData::LoadChannelList(void) 
{
  Json::Value jsonData;
  GetJson( GetChannelFile(),jsonData);

  m_channels.clear();

  int numChannels=jsonData.size();

  for( int i=0; i < numChannels; i++ ){
    Json::Value entry;
    entry = jsonData[i];
    PVR_CHANNEL channel;
    memset(&channel, 0 , sizeof(channel));
    channel.iUniqueId = entry["id"].asInt();
    channel.iChannelNumber = entry["number"].asInt();
    strncpy(channel.strChannelName, XBMC->UnknownToUTF8( entry["name"].asCString() ), sizeof(channel.strChannelName) - 1);
    strncpy(channel.strStreamURL, entry["streamURL"].asCString(), sizeof(channel.strStreamURL) - 1);
    strncpy(channel.strIconPath, entry["icon"].asCString(), sizeof(channel.strIconPath) - 1);
    m_channels.push_back(channel);
  }

  XBMC->Log(LOG_NOTICE, "Loaded %d channels.", m_channels.size());
  return true;
}

bool A1TVData::GetJson(std::string url, Json::Value &response){
  std::string strJson;
  Json::Reader jsonReader;
  GetFileContents(url,strJson);
  return jsonReader.parse(strJson, response);
}

void A1TVData::DeepCopyChannel(PVR_CHANNEL &src, PVR_CHANNEL &dst){
  memset(&dst, 0 , sizeof(dst));
  dst.iUniqueId=src.iUniqueId;
  dst.iChannelNumber=src.iChannelNumber;
  strncpy(dst.strChannelName, src.strChannelName, sizeof(dst.strChannelName) - 1);
  strncpy(dst.strStreamURL, src.strStreamURL, sizeof(dst.strStreamURL) - 1);
  strncpy(dst.strIconPath, src.strIconPath, sizeof(dst.strIconPath) - 1);
}

int A1TVData::GetChannelsAmount(void)
{
  return m_channels.size();
}

PVR_ERROR A1TVData::GetChannels(ADDON_HANDLE handle, bool bRadio)
{
  LoadChannelList();

  for ( int i = 0; i < m_channels.size(); i++)
  {
    PVR_CHANNEL tmp;
    DeepCopyChannel(m_channels.at(i), tmp);
    PVR->TransferChannelEntry(handle, &tmp );
  }

  return PVR_ERROR_NO_ERROR;
}

bool A1TVData::GetChannel(const PVR_CHANNEL &channel, PVR_CHANNEL &myChannel)
{
  return false;
}


PVR_ERROR A1TVData::GetEPGForChannel(ADDON_HANDLE handle, const PVR_CHANNEL &channel, time_t iStart, time_t iEnd)
{
  Json::Value jsonData;
  struct tm *startTime;
  startTime= localtime(&iStart);

  char tmp[256];
  // 20130811T2300/2H
  snprintf(tmp, 256, "http://epggw.a1.net/a/api.mobile.event.list?type=JSON.1&stationuid=%d&period=%04d%02d%02dT%02d%02d/%dH",
               channel.iUniqueId, //specify the channel we want
               startTime->tm_year + 1900, startTime->tm_mon + 1, startTime->tm_mday, startTime->tm_hour, startTime->tm_min, //period current time
               (int)ceil(difftime(iEnd,iStart)/3600));
  std::string url = tmp;
  if( !GetJson( url,jsonData) ){
    XBMC->Log(LOG_ERROR, "JSON request/parsing of epg data went wrong :(");
    return PVR_ERROR_FAILED;
  }
  int station_hack=0;
  Json::Value entries=jsonData["data"][station_hack]["Station"]["Events"];
  int numEntries=entries.size();

  XBMC->Log(LOG_NOTICE,"json is fetched and parsed. has to be dissected now. number of entries: %d",numEntries);

  for( int i=0; i < numEntries; i++ ){
    Json::Value entry = entries[i];

    EPG_TAG broadcast;
    memset(&broadcast, 0 , sizeof(broadcast));

    if( !entry["Title"] || !entry["ID"] || !entry["StartTime"] || !entry["EndTime"] ){
          XBMC->Log(LOG_ERROR, "json data missing/corrupted");
          return PVR_ERROR_FAILED;
    } 

    broadcast.iUniqueBroadcastId = (unsigned int)strtol(entry["ID"].asCString(),NULL,0); 
    broadcast.strTitle = XBMC->UnknownToUTF8( entry["Title"].asCString() );
    broadcast.iChannelNumber = channel.iChannelNumber;
    broadcast.startTime = ParseEPGDate(entry["StartTime"].asCString());
   // XBMC->Log(LOG_NOTICE, "parsing epgdate: %d", ParseEPGDate(entry["StartTime"].asCString()));
    broadcast.endTime = ParseEPGDate(entry["EndTime"].asCString());
  //  XBMC->Log(LOG_NOTICE, "parsing epgdate: %d", ParseEPGDate(entry["EndTime"].asCString()));

    //genre hacks
    //broadcast.iGenreType=EPG_GENRE_USE_STRING;
    //broadcast.strGenreDescription=XBMC->UnknownToUTF8( entry["Genre"].asCString() );
    //broadcast.strGenreDescription=entry.get("Genre","None").asCString();
    //broadcast.iGenreType=EPG_EVENT_CONTENTMASK_MOVIEDRAMA;
    broadcast.iGenreType=GetGenreType(entry["Genre"].asString());
    if( broadcast.iGenreType == EPG_GENRE_USE_STRING ){
      XBMC->Log(LOG_NOTICE, "unrecognized genre: %s. using string fallback!", entry["Genre"].asCString());
      broadcast.strGenreDescription=XBMC->UnknownToUTF8( entry["Genre"].asCString() );
    }

    //not implemented in xbmc yet:
    //broadcast.strEpisodeName=entry.get("SubTitle", "").asCString();
    broadcast.strIconPath=entry.get("Image", "").asCString();
    
    PVR->TransferEpgEntry(handle, &broadcast);

  }
  return PVR_ERROR_NO_ERROR;
}

int A1TVData::GetGenreType( std::string strGenre)
{
  if( genre_map.find(strGenre) != genre_map.end() ){
    return genre_map[strGenre];
  }
  return EPG_GENRE_USE_STRING;
}

time_t A1TVData::ParseEPGDate(CStdString strDate)
{
 /*
  "StartTime": "2013-08-12T00:35",
  "EndTime": "2013-08-12T01:15",
  */
  time_t rawtime;
  struct tm *tmptime;
  int year, month, day, hour, minute;

  sscanf(strDate, "%d-%d-%dT%d:%d",&year,&month,&day,&hour,&minute);
  time(&rawtime);
  tmptime = localtime(&rawtime);
  tmptime->tm_year=year-1900;
  tmptime->tm_mon=month-1;
  tmptime->tm_mday=day;
  tmptime->tm_hour=hour;
  tmptime->tm_min=minute;
  tmptime->tm_sec=0;
  return mktime(tmptime);
}

int A1TVData::GetFileContents(std::string url, std::string &strContent)
{
  strContent.clear();
  void* fileHandle = XBMC->OpenFile(url.c_str(), 0);
  if (fileHandle)
  {
    char buffer[1024];
    while (int bytesRead = XBMC->ReadFile(fileHandle, buffer, 1024))
      strContent.append(buffer, bytesRead);
    XBMC->CloseFile(fileHandle);
  }

  return strContent.length();
}


CStdString A1TVData::ReadMarkerValue(std::string &strLine, const char* strMarkerName)
{
  int iMarkerStart = (int) strLine.find(strMarkerName);
  if (iMarkerStart >= 0)
  {
    std::string strMarker = strMarkerName;
    iMarkerStart += strMarker.length();
    if (iMarkerStart < (int)strLine.length())
    {
      char cFind = ' ';
      if (strLine[iMarkerStart] == '"')
      {
        cFind = '"';
        iMarkerStart++;
      }
      int iMarkerEnd = (int)strLine.find(cFind, iMarkerStart);
      if (iMarkerEnd < 0)
      {
        iMarkerEnd = strLine.length();
      }
      return strLine.substr(iMarkerStart, iMarkerEnd - iMarkerStart);
    }
  }

  return std::string("");
}

std::string A1TVData::GetChannelFile() const
{
  string settingFile = g_strClientPath;
  if (settingFile.at(settingFile.size() - 1) == '\\' ||
      settingFile.at(settingFile.size() - 1) == '/')
    settingFile.append("channels.json");
  else
    settingFile.append("/channels.json");
  return settingFile;
}
