/*******************************************************************************
 * c7a/communication/communication_manager.cpp
 *
 ******************************************************************************/

#include "communication_manager.hpp"

namespace c7a {
namespace communication {

void CommunicationManager::Initialize(std::vector<ExecutionEndpoint> endpoints)
{
    (void) endpoints;
}

SystemControlChannel *CommunicationManager::GetSystemControlChannel()
{
	return NULL;
}

FlowControlChannel *CommunicationManager::GetFlowControlChannel()
{
	return NULL;
}
}}