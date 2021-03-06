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
 * @file Request.cc
 * @author Brian Geffon
 * @author Manjesh Nilange
 */

#include "atscppapi/Request.h"
#include <ts/ts.h>
#include "atscppapi/noncopyable.h"
#include "InitializableValue.h"
#include "utils_internal.h"
#include "logging_internal.h"

using namespace atscppapi;
using std::string;

/**
 * @private
 */
struct atscppapi::RequestState: noncopyable  {
  TSMBuffer hdr_buf_;
  TSMLoc hdr_loc_;
  TSMLoc url_loc_;
  Url url_;
  Headers headers_;
  InitializableValue<HttpMethod> method_;
  InitializableValue<HttpVersion> version_;
  bool destroy_buf_;
  RequestState() : hdr_buf_(NULL), hdr_loc_(NULL), url_loc_(NULL), method_(HTTP_METHOD_UNKNOWN, false),
                   version_(HTTP_VERSION_UNKNOWN, false), destroy_buf_(false) { }
};

Request::Request() {
  state_ = new RequestState();
}

Request::Request(void *hdr_buf, void *hdr_loc) {
  state_ = new RequestState();
  init(hdr_buf, hdr_loc);
  LOG_DEBUG("Initialized request object %p with hdr_buf=%p and hdr_loc=%p", this, hdr_buf, hdr_loc);
}

Request::Request(const string &url_str, HttpMethod method, HttpVersion version) {
  state_ = new RequestState();
  state_->method_.setValue(method);
  state_->version_.setValue(version);
  state_->destroy_buf_ = true;
  state_->hdr_buf_ = TSMBufferCreate();
  state_->headers_.initDetached();
  if (TSUrlCreate(state_->hdr_buf_, &state_->url_loc_) == TS_SUCCESS) {
    const char *url_str_start = url_str.c_str();
    const char *url_str_end = url_str_start + url_str.size();
    if (TSUrlParse(state_->hdr_buf_, state_->url_loc_, &url_str_start, url_str_end) != TS_PARSE_DONE) {
      LOG_ERROR("[%s] does not represent a valid url", url_str.c_str());
    }
    else {
      state_->url_.init(state_->hdr_buf_, state_->url_loc_);
    }
  }
  else {
    state_->url_loc_ = NULL;
    LOG_ERROR("Could not create URL field; hdr_buf %p", state_->hdr_buf_); 
  }
}

void Request::init(void *hdr_buf, void *hdr_loc) {
  if (state_->hdr_buf_ || state_->hdr_loc_) {
    LOG_ERROR("Reinitialization; (hdr_buf, hdr_loc) current(%p, %p), attempted(%p, %p)", state_->hdr_buf_,
              state_->hdr_loc_, hdr_buf, hdr_loc);
    return;
  }
  state_->hdr_buf_ = static_cast<TSMBuffer>(hdr_buf);
  state_->hdr_loc_ = static_cast<TSMLoc>(hdr_loc);
  state_->headers_.init(state_->hdr_buf_, state_->hdr_loc_);
  state_->url_loc_ = NULL;
  TSHttpHdrUrlGet(state_->hdr_buf_, state_->hdr_loc_, &state_->url_loc_);
  if (!state_->url_loc_) {
    LOG_ERROR("TSHttpHdrUrlGet returned a null url loc, hdr_buf=%p, hdr_loc=%p", state_->hdr_buf_, state_->hdr_loc_);
  }
  else {
    state_->url_.init(state_->hdr_buf_, state_->url_loc_);
    LOG_DEBUG("Initialized url");
  }
}

HttpMethod Request::getMethod() const {
  if (!state_->method_.isInitialized() && state_->hdr_buf_ && state_->hdr_loc_) {
    int method_len;
    const char *method_str = TSHttpHdrMethodGet(state_->hdr_buf_, state_->hdr_loc_, &method_len);
    if (method_str && method_len) {
      if (method_str == TS_HTTP_METHOD_GET) {
        state_->method_ = HTTP_METHOD_GET;
      } else if (method_str == TS_HTTP_METHOD_POST) {
        state_->method_ = HTTP_METHOD_POST;
      } else if (method_str == TS_HTTP_METHOD_HEAD) {
        state_->method_ = HTTP_METHOD_HEAD;
      } else if (method_str == TS_HTTP_METHOD_CONNECT) {
        state_->method_ = HTTP_METHOD_CONNECT;
      } else if (method_str == TS_HTTP_METHOD_DELETE) {
        state_->method_ = HTTP_METHOD_DELETE;
      } else if (method_str == TS_HTTP_METHOD_ICP_QUERY) {
        state_->method_ = HTTP_METHOD_ICP_QUERY;
      } else if (method_str == TS_HTTP_METHOD_OPTIONS) {
        state_->method_ = HTTP_METHOD_OPTIONS;
      } else if (method_str == TS_HTTP_METHOD_PURGE) {
        state_->method_ = HTTP_METHOD_PURGE;
      } else if (method_str == TS_HTTP_METHOD_PUT) {
        state_->method_ = HTTP_METHOD_PUT;
      } else if (method_str == TS_HTTP_METHOD_TRACE) {
        state_->method_ = HTTP_METHOD_TRACE;
      }
    } else {
      LOG_ERROR("TSHttpHdrMethodGet returned null string or it was zero length, hdr_buf=%p, hdr_loc=%p, method str=%p, method_len=%d",
          state_->hdr_buf_, state_->hdr_loc_, method_str, method_len);
    }
  }
  return state_->method_;
}

Url &Request::getUrl() {
  return state_->url_;
}

atscppapi::HttpVersion Request::getVersion() const {
  if (!state_->version_.isInitialized() && state_->hdr_buf_ && state_->hdr_loc_) {
    state_->version_ = utils::internal::getHttpVersion(state_->hdr_buf_, state_->hdr_loc_);
    LOG_DEBUG("Initializing request version=%d [%s] on hdr_buf=%p, hdr_loc=%p",
        state_->version_.getValue(), HTTP_VERSION_STRINGS[state_->version_.getValue()].c_str(), state_->hdr_buf_, state_->hdr_loc_);
  }
  return state_->version_;
}

atscppapi::Headers &Request::getHeaders() const {
  return state_->headers_;
}

Request::~Request() {
  if (state_->url_loc_) {
    if (state_->destroy_buf_) {
      // usually, hdr_loc is the parent of url_loc, but we created this url_loc "directly" in hdr_buf, 
      // so we use null as parent loc in this case
      TSMLoc null_parent_loc = NULL;
      TSHandleMLocRelease(state_->hdr_buf_, null_parent_loc, state_->url_loc_);
      TSMBufferDestroy(state_->hdr_buf_);
    } else {
      LOG_DEBUG("Destroying request object on hdr_buf=%p, hdr_loc=%p, url_loc=%p", state_->hdr_buf_,
                state_->hdr_loc_, state_->url_loc_);
      TSHandleMLocRelease(state_->hdr_buf_, state_->hdr_loc_, state_->url_loc_);
    }
  }
  delete state_;
}
