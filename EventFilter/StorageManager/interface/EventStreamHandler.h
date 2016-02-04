// $Id: EventStreamHandler.h,v 1.6 2010/03/19 13:24:30 mommsen Exp $
/// @file: EventStreamHandler.h 

#ifndef StorageManager_EventStreamHandler_h
#define StorageManager_EventStreamHandler_h

#include <string>

#include "IOPool/Streamer/interface/InitMessage.h"

#include "EventFilter/StorageManager/interface/InitMsgCollection.h"
#include "EventFilter/StorageManager/interface/StreamHandler.h"


namespace stor {

  class Configuration;
  class EventStreamConfigurationInfo;


  /**
   * Handle one event stream written to disk.
   *
   * $Author: mommsen $
   * $Revision: 1.6 $
   * $Date: 2010/03/19 13:24:30 $
   */
  
  class EventStreamHandler : public StreamHandler
  {
  public:
    
    EventStreamHandler
    (
      const EventStreamConfigurationInfo&,
      const SharedResourcesPtr,
      const DbFileHandlerPtr
    );


  private:

    /**
     * Return the stream label
     */
    virtual std::string streamLabel() const
    { return _streamConfig.streamLabel(); }

    /**
     * Return the fraction-to-disk parameter
     */
    virtual double fractionToDisk() const
    { return _streamConfig.fractionToDisk(); }

    /**
     * Return a new file handler for the provided event
     */    
    virtual FileHandlerPtr newFileHandler(const I2OChain& event);

    /**
     * Return the maximum file size for the stream in MB
     */
    virtual int getStreamMaxFileSize() const
    { return _streamConfig.maxFileSizeMB(); }


    EventStreamConfigurationInfo _streamConfig;
    boost::shared_ptr<InitMsgCollection> _initMsgCollection;
    InitMsgSharedPtr _initMsgView;

  };
  
} // namespace stor

#endif // StorageManager_EventStreamHandler_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -