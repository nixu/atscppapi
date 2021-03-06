/*
 * Copyright (c) 2013 LinkedIn Corp. All rights reserved.
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of the license at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the
 * License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied.
 *
 */

/**
 * @file Plugin.h
 * @author Brian Geffon
 * @author Manjesh Nilange
 *
 * @brief Contains the base interface used in creating Global and Transaciton plugins.
 * \note This interface can never be implemented directly, it should be implemented
 *   through extending GlobalPlugin, TransactionPlugin, or TransformationPlugin.
 */

#pragma once
#ifndef ATSCPPAPI_PLUGIN_H_
#define ATSCPPAPI_PLUGIN_H_

#include <atscppapi/Transaction.h>
#include <atscppapi/noncopyable.h>

namespace atscppapi {

/**
 * @brief The base interface used when creating a Plugin.
 *
 * \note This interface can never be implemented directly, it should be implemented
 *   through extending GlobalPlugin, TransactionPlugin, or TransformationPlugin.
 *
 * @see TransactionPlugin
 * @see GlobalPlugin
 * @see TransformationPlugin
 */
class Plugin: noncopyable {
public:
  /**
   * A enumeration of the available types of Hooks. These are used with GlobalPlugin::registerHook()
   * and TransactionPlugin::registerHook().
   */
  enum HookType {
    HOOK_READ_REQUEST_HEADERS_PRE_REMAP = 0, /**< This hook will be fired before remap has occured. */
    HOOK_READ_REQUEST_HEADERS_POST_REMAP, /**< This hook will be fired directly after remap has occured. */
    HOOK_SEND_REQUEST_HEADERS, /**< This hook will be fired right before request headers are sent to the origin */
    HOOK_READ_RESPONSE_HEADERS, /**< This hook will be fired right after response headers have been read from the origin */
    HOOK_SEND_RESPONSE_HEADERS, /**< This hook will be fired right before the response headers are sent to the client */
    HOOK_OS_DNS /**< This hook will be fired right after the OS DNS lookup */
  };

  /**
   * This method must be implemented when you hook HOOK_READ_REQUEST_HEADERS_PRE_REMAP
   */
  virtual void handleReadRequestHeadersPreRemap(Transaction &transaction) { transaction.resume(); };

  /**
   * This method must be implemented when you hook HOOK_READ_REQUEST_HEADERS_POST_REMAP
   */
  virtual void handleReadRequestHeadersPostRemap(Transaction &transaction) { transaction.resume(); };

  /**
   * This method must be implemented when you hook HOOK_SEND_REQUEST_HEADERS
   */
  virtual void handleSendRequestHeaders(Transaction &transaction) { transaction.resume(); };

  /**
   * This method must be implemented when you hook HOOK_READ_RESPONSE_HEADERS
   */
  virtual void handleReadResponseHeaders(Transaction &transaction) { transaction.resume(); };

  /**
   * This method must be implemented when you hook HOOK_SEND_RESPONSE_HEADERS
   */
  virtual void handleSendResponseHeaders(Transaction &transaction) { transaction.resume(); };

  /**
   * This method must be implemented when you hook HOOK_OS_DNS
   */
  virtual void handleOsDns(Transaction &transaction) { transaction.resume(); };

  virtual ~Plugin() { };
protected:
  /**
  * \note This interface can never be implemented directly, it should be implemented
  *   through extending GlobalPlugin, TransactionPlugin, or TransformationPlugin.
  *
  * @private
  */
  Plugin() { };
};

/**< Human readable strings for each HookType, you can access them as HOOK_TYPE_STRINGS[HOOK_OS_DNS] for example. */
extern const std::string HOOK_TYPE_STRINGS[];

} /* atscppapi */

#endif /* ATSCPPAPI_GLOBALPLUGIN_H_ */
