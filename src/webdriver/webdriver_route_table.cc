// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webdriver_route_table.h"

#include <sstream>
#include <string.h>
#include <stdlib.h>
#include <vector>

#include "webdriver_route_patterns.h"
#include "commands/non_session_commands.h"
#include "commands/create_session.h"
#include "commands/session_with_id.h"
#include "commands/sessions.h"
#include "commands/set_timeout_commands.h"
#include "commands/log_command.h"
#include "commands/window_commands.h"
#include "commands/title_command.h"
#include "commands/navigate_commands.h"
#include "commands/screenshot_command.h"
#include "commands/keys_command.h"
#include "commands/mouse_commands.h"
#include "commands/webelement_commands.h"
#include "webdriver_logging.h"
#include "base/string_split.h"


namespace webdriver {


RouteTable::RouteTable(){
}

RouteTable::RouteTable(const RouteTable &obj)
    : routes_(obj.routes_) {
}
 
RouteTable& RouteTable::operator= (const RouteTable &obj) {
    routes_ = obj.routes_;
    return *this;
}

RouteTable::~RouteTable() {}

void RouteTable::Remove(const std::string& pattern) {
    std::vector<webdriver::internal::RouteDetails>::iterator route;
    for (route = routes_.begin();
         route < routes_.end();
         ++route) {
        if (pattern == route->uri_regex_) {
            routes_.erase(route);
            break;
        }
    }
}

bool RouteTable::HasRoute(const std::string& pattern) {
    std::vector<webdriver::internal::RouteDetails>::const_iterator route;
    for (route = routes_.begin();
         route < routes_.end();
         ++route) {
        if (pattern == route->uri_regex_) {
            return true;
        }
    }
    return false;
}

void RouteTable::Clear() {
    routes_.clear();
}

std::vector<std::string> RouteTable::GetRoutes() {
    std::vector<std::string> routes_to_ret;

    std::vector<webdriver::internal::RouteDetails>::const_iterator route;
    for (route = routes_.begin();
         route < routes_.end();
         ++route) {
        routes_to_ret.push_back(route->uri_regex_);
    }

    return routes_to_ret;
}

CommandCreatorPtr RouteTable::GetRouteForURL(const std::string& url) {
    std::vector<webdriver::internal::RouteDetails>::const_iterator route;
    for (route = routes_.begin();
         route < routes_.end();
         ++route) {
        if (MatchPattern(url, route->uri_regex_)) {
            return route->creator_;
        }
    }

    return NULL;
}


void RouteTable::AddRoute(const std::string& uri_pattern,
                          const CommandCreatorPtr& creator) {
    // custom command check
    if (!CommandRoutes::IsStandardRoute(uri_pattern)) {
        // TODO: validate custom command syntax
    }

    std::vector<webdriver::internal::RouteDetails>::iterator route;
    for (route = routes_.begin();
         route < routes_.end();
         ++route) {
        if (route->uri_regex_ == uri_pattern) {
            // replace command for pattern
            *route = webdriver::internal::RouteDetails(uri_pattern, creator);
            return;
        }

        if (CompareBestMatch(uri_pattern, route->uri_regex_)) {
            // put best match pattern before other
            routes_.insert(route, webdriver::internal::RouteDetails(uri_pattern, creator));
            return;
        }
    }

    routes_.push_back(webdriver::internal::RouteDetails(
                         uri_pattern,
                         creator));
}

bool RouteTable::MatchPattern(const std::string& url, const std::string& pattern) {
    std::vector<std::string> url_segments;
    std::vector<std::string> pattern_segments;

    base::SplitString(url, '/', &url_segments);
    base::SplitString(pattern, '/', &pattern_segments);

    unsigned int segments_num = url_segments.size();

    if (segments_num != pattern_segments.size()) {
        // different segments num
        return false;
    }

    for (unsigned int i = 0; i < segments_num; i++) {
        if (pattern_segments.at(i) == "*")
            continue;
        if (pattern_segments.at(i) != url_segments.at(i))
            return false;
    }

    return true;
}

bool RouteTable::CompareBestMatch(const std::string& uri_pattern1, const std::string& uri_pattern2) { 
    // TODO: implement
    return false;
}

DefaultRouteTable::DefaultRouteTable() 
    : RouteTable() {

    // Place default commands registration here
    Add<StatusCommand>(CommandRoutes::kStatus);
    Add<GlobalLogCommand>(CommandRoutes::kGetLog);
    Add<CreateSession>(CommandRoutes::kNewSession);
    Add<SessionWithID>(CommandRoutes::kSession);
    Add<Sessions>(CommandRoutes::kSessions);
    Add<SetAsyncScriptTimeoutCommand>(CommandRoutes::kSetScriptTimeout);
    Add<ImplicitWaitCommand>(CommandRoutes::kImplicitlyWait);
    Add<LogTypesCommand>(CommandRoutes::kGetAvailableLogTypes);
    Add<LogCommand>(CommandRoutes::kGetSessionLogs);
    Add<WindowSizeCommand>(CommandRoutes::kWindowSize);
    Add<WindowPositionCommand>(CommandRoutes::kWindowPosition);
    Add<WindowMaximizeCommand>(CommandRoutes::kMaximizeWindow);
    Add<TitleCommand>(CommandRoutes::kGetTitle);
    Add<ForwardCommand>(CommandRoutes::kGoForward);
    Add<BackCommand>(CommandRoutes::kGoBack);
    Add<RefreshCommand>(CommandRoutes::kRefresh);
    Add<ScreenshotCommand>(CommandRoutes::kScreenshot);
    Add<KeysCommand>(CommandRoutes::kSendKeys);
    Add<MoveAndClickCommand>(CommandRoutes::kClickElement);
    Add<HoverCommand>(CommandRoutes::kHoverOverElement);
    Add<ClickCommand>(CommandRoutes::kMouseClick);
    Add<DoubleClickCommand>(CommandRoutes::kMouseDoubleClick);
    Add<ButtonDownCommand>(CommandRoutes::kMouseButtonDown);
    Add<ButtonUpCommand>(CommandRoutes::kMouseButtonUp);
    Add<MoveToCommand>(CommandRoutes::kMouseMoveTo);

    Add<ElementClearCommand>(CommandRoutes::kClearElement);
    Add<ElementValueCommand>(CommandRoutes::kSendKeysToElement);
    Add<ElementSubmitCommand>(CommandRoutes::kSubmitElement);
    Add<ElementTextCommand>(CommandRoutes::kGetElementText);
    Add<ElementNameCommand>(CommandRoutes::kGetElementTagName);
    Add<ElementSelectedCommand>(CommandRoutes::kIsElementSelected);
    Add<ElementEnabledCommand>(CommandRoutes::kIsElementEnabled);
    Add<ElementDisplayedCommand>(CommandRoutes::kIsElementDisplayed);
    Add<ElementLocationCommand>(CommandRoutes::kGetElementLocation);
    Add<ElementLocationInViewCommand>(CommandRoutes::kGetElementLocationInView);
    Add<ElementSizeCommand>(CommandRoutes::kGetElementSize);
    Add<ElementAttributeCommand>(CommandRoutes::kGetElementAttribute);
    Add<ElementCssCommand>(CommandRoutes::kGetElementValueOfCssProperty);
    Add<ElementEqualsCommand>(CommandRoutes::kElementEquals);


#if 0
dispatcher->AddShutdown("/shutdown", shutdown_event);

  dispatcher->AddLog("/log");

  dispatcher->Add<CreateSession>("/session");

  dispatcher->Add<Sessions>("/sessions");

  // WebElement commands
  dispatcher->Add<FindOneElementCommand>(  "/session/*/element");
  dispatcher->Add<FindManyElementsCommand>("/session/*/elements");
  dispatcher->Add<ActiveElementCommand>(   "/session/*/element/active");
  dispatcher->Add<FindOneElementCommand>(  "/session/*/element/*/element");
  dispatcher->Add<FindManyElementsCommand>("/session/*/elements/*/elements");
  


  // Mouse Commands

  // All session based commands should be listed after the element based
  // commands to avoid potential mapping conflicts from an overzealous
  // wildcard match. For example, /session/*/title maps to the handler to
  // fetch the page title. If mapped first, this would overwrite the handler
  // for /session/*/element/*/attribute/title, which should fetch the title
  // attribute of the element.
  dispatcher->Add<AcceptAlertCommand>(  "/session/*/accept_alert");
  dispatcher->Add<AlertTextCommand>(    "/session/*/alert_text");
    dispatcher->Add<DismissAlertCommand>( "/session/*/dismiss_alert");
  dispatcher->Add<ExecuteCommand>(      "/session/*/execute");
  dispatcher->Add<ExecuteAsyncScriptCommand>(
                                        "/session/*/execute_async");
  
  dispatcher->Add<SwitchFrameCommand>(  "/session/*/frame");
  
  dispatcher->Add<SourceCommand>(       "/session/*/source");
  dispatcher->Add<URLCommand>(          "/session/*/url");
  dispatcher->Add<WindowCommand>(       "/session/*/window");
  dispatcher->Add<WindowHandleCommand>( "/session/*/window_handle");
  dispatcher->Add<WindowHandlesCommand>("/session/*/window_handles");

  // Cookie functions.
  dispatcher->Add<CookieCommand>(     "/session/*/cookie");
  dispatcher->Add<NamedCookieCommand>("/session/*/cookie/*");

  dispatcher->Add<BrowserConnectionCommand>("/session/*/browser_connection");
  dispatcher->Add<AppCacheStatusCommand>("/session/*/application_cache/status");

  // HTML5 functions.
  dispatcher->Add<HTML5LocationCommand>("/session/*/location");
  dispatcher->Add<LocalStorageCommand>("/session/*/local_storage");
  dispatcher->Add<LocalStorageSizeCommand>("/session/*/local_storage/size");
  dispatcher->Add<LocalStorageKeyCommand>("/session/*/local_storage/key*");
  dispatcher->Add<SessionStorageCommand>("/session/*/session_storage");
  dispatcher->Add<SessionStorageSizeCommand>("/session/*/session_storage/size");
  dispatcher->Add<SessionStorageKeyCommand>("/session/*/session_storage/key*");
#endif  
  
}

DefaultRouteTable::~DefaultRouteTable() {}

}  // namespace webdriver
