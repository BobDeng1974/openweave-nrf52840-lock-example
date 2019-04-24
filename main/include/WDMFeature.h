/*
 *
 *    Copyright (c) 2019 Nest Labs, Inc.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#ifndef WDM_FEATURE_H
#define WDM_FEATURE_H

#include <Weave/DeviceLayer/WeaveDeviceLayer.h>

class WDMFeature:
{
	typedef ::nl::Weave::Profiles::DataManagement_Current::SubscriptionClient SubscriptionClient;
    typedef ::nl::Weave::Profiles::DataManagement_Current::SubscriptionEngine SubscriptionEngine;
    typedef ::nl::Weave::Profiles::DataManagement_Current::SubscriptionHandler SubscriptionHandler;
    typedef ::nl::Weave::Profiles::DataManagement_Current::TraitDataSink TraitDataSink;
    typedef ::nl::Weave::Profiles::DataManagement_Current::TraitDataSource TraitDataSource;
    typedef ::nl::Weave::Profiles::DataManagement_Current::TraitPath TraitPath;
    typedef ::nl::Weave::Profiles::DataManagement_Current::ResourceIdentifier ResourceIdentifier;
    typedef ::nl::Weave::Profiles::DataManagement_Current::PropertyPathHandle PropertyPathHandle;

public:

    WEAVE_ERROR Init(uint32_t intervalMS);
    WEAVW_ERROR InitiateSubscriptionToService(void);

private:
    static void PlatformEventHandler(const ::nl::Weave::DeviceLayer::WeaveDeviceEvent * event, intptr_t arg);
    static void HandleSubscriptionEngineEvent(void * appState, SubscriptionEngine::EventID eventType,
            const SubscriptionEngine::InEventParam & inParam, SubscriptionEngine::OutEventParam & outParam);
    static void HandleServiceBindingEvent(void * appState, ::nl::Weave::Binding::EventType eventType,
        const ::nl::Weave::Binding::InEventParam & inParam, ::nl::Weave::Binding::OutEventParam & outParam);
    static void HandleOutboundServiceSubscriptionEvent(void * appState, SubscriptionClient::EventID eventType,
            const SubscriptionClient::InEventParam & inParam, SubscriptionClient::OutEventParam & outParam);
    static void HandleInboundSubscriptionEvent(void * aAppState, SubscriptionHandler::EventID eventType,
            const SubscriptionHandler::InEventParam & inParam, SubscriptionHandler::OutEventParam & outParam);

	SubscriptionClient * mServiceSubClient;
    SubscriptionHandler * mServiceCounterSubHandler;
    TraitPath * mServicePathList;

    TraitSinkCatalog mSubscribedServiceTraits;
	TraitSourceCatalog mPublishedTraits;

	SubscriptionEngine mWdmSubscriptionEngine;
};

#endif // WDM_FEATURE_H 
