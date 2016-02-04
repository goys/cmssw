// $Id: Starting.cc,v 1.10 2010/08/06 20:24:31 wmtan Exp $
/// @file: Starting.cc

#include "EventFilter/StorageManager/interface/CommandQueue.h"
#include "EventFilter/StorageManager/interface/Configuration.h"
#include "EventFilter/StorageManager/interface/DiskWriterResources.h"
#include "EventFilter/StorageManager/interface/DQMEventProcessorResources.h"
#include "EventFilter/StorageManager/interface/ErrorStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/EventStreamConfigurationInfo.h"
#include "EventFilter/StorageManager/interface/SharedResources.h"
#include "EventFilter/StorageManager/interface/StateMachine.h"
#include "EventFilter/StorageManager/interface/TransitionRecord.h"

#include <iostream>
#include <unistd.h>

using namespace std;
using namespace stor;

Starting::Starting( my_context c ): my_base(c)
{
  safeEntryAction();
}

void Starting::do_entryActionWork()
{
  TransitionRecord tr( stateName(), true );
  outermost_context().updateHistory( tr );

  SharedResourcesPtr sharedResources =
    outermost_context().getSharedResources();

  // Request DiskWriter to configure streams
  EvtStrConfigListPtr evtCfgList = sharedResources->_configuration->
    getCurrentEventStreamConfig();
  ErrStrConfigListPtr errCfgList = sharedResources->_configuration->
    getCurrentErrorStreamConfig();

  WorkerThreadParams workerParams =
    sharedResources->_configuration->getWorkerThreadParams();
  sharedResources->_diskWriterResources->
    requestStreamConfiguration(evtCfgList, errCfgList,
      sharedResources->_configuration->getDiskWritingParams(),
      sharedResources->_configuration->getRunNumber(),
      workerParams._DWdeqWaitTime);

  // Request configuration of DQMEventProcessor
  sharedResources->_dqmEventProcessorResources->
    requestConfiguration(
      sharedResources->_configuration->getDQMProcessingParams(),
      workerParams._DQMEPdeqWaitTime);
}

Starting::~Starting()
{
  safeExitAction();
}

void Starting::do_exitActionWork()
{
  TransitionRecord tr( stateName(), false );
  outermost_context().updateHistory( tr );
}

string Starting::do_stateName() const
{
  return std::string( "Starting" );
}

void Starting::do_moveToFailedState( xcept::Exception& exception ) const
{
  outermost_context().getSharedResources()->moveToFailedState( exception );
}

void Starting::logStopDoneRequest( const StopDone& request )
{
  outermost_context().unconsumed_event( request );
}

void Starting::logHaltDoneRequest( const HaltDone& request )
{
  outermost_context().unconsumed_event( request );
}

void
Starting::do_noFragmentToProcess() const
{
  if ( workerThreadsConfigured() )
  {
    SharedResourcesPtr sharedResources =
      outermost_context().getSharedResources();
    event_ptr stMachEvent( new StartRun() );
    sharedResources->_commandQueue->enq_wait( stMachEvent );
  }
}

bool
Starting::workerThreadsConfigured() const
{
  SharedResourcesPtr sharedResources =
    outermost_context().getSharedResources();

  // check if the requests are still being processed
  if ( sharedResources->_diskWriterResources->streamChangeOngoing() ) return false;

  if ( sharedResources->_dqmEventProcessorResources->requestsOngoing() ) return false;

  return true; 
}


/// emacs configuration
/// Local Variables: -
/// mode: c++ -
/// c-basic-offset: 2 -
/// indent-tabs-mode: nil -
/// End: -