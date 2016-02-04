// $Id: EventStreamHandler.cc,v 1.7 2010/05/11 18:02:29 mommsen Exp $
/// @file: EventStreamHandler.cc

#include "EventFilter/StorageManager/interface/Configuration.h"
#include "EventFilter/StorageManager/interface/EventFileHandler.h"
#include "EventFilter/StorageManager/interface/EventStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/EventStreamHandler.h"


using namespace stor;


EventStreamHandler::EventStreamHandler
(
  const EventStreamConfigurationInfo& streamConfig,
  const SharedResourcesPtr sharedResources,
  const DbFileHandlerPtr dbFileHandler
):
StreamHandler(sharedResources, dbFileHandler),
_streamConfig(streamConfig),
_initMsgCollection(sharedResources->_initMsgCollection)
{
  _streamRecord->streamName = streamLabel();
  _streamRecord->fractionToDisk = fractionToDisk();
}


StreamHandler::FileHandlerPtr
EventStreamHandler::newFileHandler(const I2OChain& event)
{
  // the INIT message is not available when the EventStreamHandler is
  // constructed, so we need to fetch it when we first need a new file
  // handler (when the first event is received, which is after the 
  // INIT messages have been received)
  if (_initMsgView.get() == 0)
    {
      _initMsgView = _initMsgCollection->getElementForOutputModule(_streamConfig.outputModuleLabel());
    }

  FilesMonitorCollection::FileRecordPtr fileRecord = getNewFileRecord(event);

  FileHandlerPtr newFileHandler(
    new EventFileHandler(_initMsgView, fileRecord, _dbFileHandler,
      _diskWritingParams, getMaxFileSize())
  );
  _fileHandlers.push_back(newFileHandler);

  return newFileHandler;
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -