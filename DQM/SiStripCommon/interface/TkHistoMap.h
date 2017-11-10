#ifndef DQM_SiStripCommon_TKHistoMap_h
#define DQM_SiStripCommon_TKHistoMap_h

#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "CalibTracker/SiStripCommon/interface/TkDetMap.h"
#include "CommonTools/TrackerMap/interface/TrackerMap.h"
#include <string>

class TkHistoMap{

  typedef std::vector<MonitorElement*> tkHistoMapVect;

 public:
  TkHistoMap(DQMStore::IBooker & ibooker , const TkDetMap* tkDetMap, const std::string& path, const std::string& MapName, float baseline=0, bool mechanicalView=false);
  TkHistoMap(const TkDetMap* tkDetMap, const std::string& path, const std::string& MapName, float baseline=0, bool mechanicalView=false);
  TkHistoMap();
  ~TkHistoMap(){};

  void loadServices();

  void loadTkHistoMap(const std::string& path, const std::string& MapName, bool mechanicalView=false);

  MonitorElement* getMap(short layerNumber){return tkHistoMap_[layerNumber];};
  std::vector<MonitorElement*>& getAllMaps(){return tkHistoMap_;};

  float getValue(DetId detid);
  float getEntries(DetId detid);
  DetId getDetId(const std::string& title, int ix, int iy){return getDetId(getLayerNum(getLayerName(title)),ix,iy);}
  DetId getDetId(int layer, int ix, int iy){return tkdetmap_->getDetFromBin(layer,ix,iy);}
  DetId getDetId(const MonitorElement* ME, int ix, int iy){return getDetId(ME->getTitle(),ix,iy);}
  std::string getLayerName(std::string title){return title.erase(0,MapName_.size()+1);}
  uint16_t getLayerNum(const std::string& layerName){return tkdetmap_->getLayerNum(layerName);}

  void fillFromAscii(const std::string& filename);
  void fill(DetId detid,float value);
  void setBinContent(DetId detid,float value);
  void add(DetId detid,float value);

  void dumpInTkMap(TrackerMap* tkmap, bool dumpEntries=false); //dumpEntries==true? (dump entries) : (dump mean values)
  void save(const std::string& filename);
  void saveAsCanvas(const std::string& filename, const std::string& options="", const std::string& mode="RECREATE");

 private:

  //fixme: keep single method
  void createTkHistoMap(const std::string& path, const std::string& MapName, float baseline, bool mechanicalView);
  void createTkHistoMap(DQMStore::IBooker & ibooker , const std::string& path, const std::string& MapName, float baseline, bool mechanicalView);

  std::string folderDefinition(std::string path, const std::string& MapName, int layer , bool mechanicalView, std::string& fullName);

  DQMStore* dqmStore_;
  const TkDetMap* tkdetmap_;
  DetId cached_detid;
  int16_t cached_layer;
  TkLayerMap::XYbin cached_XYbin;
  std::vector<MonitorElement*> tkHistoMap_;
  int HistoNumber;
  std::string MapName_;
};

#endif
