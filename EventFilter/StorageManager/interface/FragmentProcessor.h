// $Id: FragmentProcessor.h,v 1.5 2010/12/10 19:38:48 mommsen Exp $
/// @file: FragmentProcessor.h 

#ifndef StorageManager_FragmentProcessor_h
#define StorageManager_FragmentProcessor_h

#include "toolbox/lang/Class.h"
#include "toolbox/task/WaitingWorkLoop.h"
#include "xdaq/Application.h"

#include "boost/date_time/posix_time/posix_time_types.hpp"
#include "boost/shared_ptr.hpp"

#include "EventFilter/StorageManager/interface/EventDistributor.h"
#include "EventFilter/StorageManager/interface/FragmentQueue.h"
#include "EventFilter/StorageManager/interface/FragmentStore.h"
#include "EventFilter/StorageManager/interface/SharedResources.h"
#include "EventFilter/StorageManager/interface/WrapperNotifier.h"


namespace stor {

  class I2OChain;
  class QueueID;
  class StateMachine;


  /**
   * Processes I2O event fragments
   *
   * It pops the next fragment from the FragmentQueue and adds it to the
   * FragmentStore. If this completes the event, it hands it to the 
   * EventDistributor.
   *
   * $Author: mommsen $
   * $Revision: 1.5 $
   * $Date: 2010/12/10 19:38:48 $
   */

  class FragmentProcessor : public toolbox::lang::Class
  {
  public:
    
    FragmentProcessor( xdaq::Application *app, SharedResourcesPtr sr );

    ~FragmentProcessor();
    
    /**
     * The workloop action processing state machine commands from the
     * command queue and handling I2O messages retrieved from the
     * FragmentQueue
     */
    bool processMessages(toolbox::task::WorkLoop*);

    /**
     * Create and start the fragment processing workloop
     */
    void startWorkLoop(std::string workloopName);


  private:

    /**
     * Processes all state machine events in the command queue
     */
    void processAllCommands();

    /**
     * Processes all consumer registrations in the registration queue
     */
    void processAllRegistrations();

    /**
       Process a single fragment, if there is  place to put it.
     */
    void processOneFragmentIfPossible();

    /**
       Process a single fragment. This should only be called if it has
       already been determined there is a place to put it.
     */
    void processOneFragment();

    xdaq::Application*                 _app;
    SharedResourcesPtr                 _sharedResources;
    WrapperNotifier                    _wrapperNotifier;
    boost::shared_ptr<StateMachine>    _stateMachine;
    FragmentStore                      _fragmentStore;
    EventDistributor                   _eventDistributor;

    boost::posix_time::time_duration   _timeout; // Waiting time
    bool                               _actionIsActive;

    toolbox::task::WorkLoop*           _processWL;      

  };
  
} // namespace stor

#endif // StorageManager_FragmentProcessor_h 


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -