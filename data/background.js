'use strict';

var global = this;
var enabledIcon = {'path': {'19': 'nyu19_on.png', '38': 'nyu38_on.png'}};
var disabledIcon = {'path': {'19': 'nyu19.png', '38': 'nyu38.png'}};
var connectedPorts = [];
var settings = {
  enabled: true,
  textColor: '#fff',
  backgroundColor: '#000',
};

// Cool trick to modify auto-generated embed page:
// See https://groups.google.com/d/msg/native-client-discuss/UJu7VXvV_bw/pLc19D50gbwJ
//
// When a new tab is created (detected by chrome.tabs.on{Created,Updated}, we
// inject a short script (via chrome.tabs.executeScript) to see if it is an
// automatically-generated page for our NaCl mimetype handler.
//
// If so, the page connects to our extension (via chrome.runtime.connect), and
// sends the background page a message. We then inject more JavaScript to
// style the page. Finally, we connected to that page (via
// chrome.tabs.connect), so we can send additional messages (e.g. if the user
// has chosen a different background color).
function injectScript(tab) {
  // This is a minimal script that only checks to see if we should be
  // injecting more.
  chrome.tabs.executeScript(tab.id, {
    file: 'inject.js',
    runAt: 'document_start'
  });
}

function isChromeExtensionURL(url) {
  return url && url.lastIndexOf('chrome-extension://', 0) === 0;
}

chrome.tabs.onCreated.addListener(function(tab) {
  if (!isChromeExtensionURL(tab.url))
    injectScript(tab);
});

chrome.tabs.onUpdated.addListener(function(tabId, changeInfo, tab) {
  if (!isChromeExtensionURL(tab.url))
    injectScript(tab);
});


chrome.runtime.onMessage.addListener(function(message, sender) {
  if (message !== 'injected') {
    console.log('Unknown message received from content script: ' +
        JSON.stringify(message));
    return;
  }

  // If the tab sent us a response, it must be a NaCl mimetype handler page.
  // Inject the full script to resize the embed, etc...
  chrome.tabs.executeScript(sender.tab.id, {
    file: 'injected.js',
    runAt: 'document_start'
  }, function() {
    var port = chrome.tabs.connect(sender.tab.id, {name: 'background'});
    // Connect to the tab port.
    onPortConnected(port);
  });
});

// Connect to an extension port.
chrome.runtime.onConnect.addListener(onPortConnected);

function onPortConnected(port) {
  console.log('onConnect called with port: ' + port.name);
  global.connectedPorts.push(port);

  var onDisconnect = function() {
    port.onMessage.removeListener(onMessage);
    port.onDisconnect.removeListener(onDisconnect);
    var index = global.connectedPorts.indexOf(port);
    if (index !== -1)
      global.connectedPorts.splice(index, 1);
  };

  var onMessage = function(message, sender) {
    console.log('onMessage called with message: ' + JSON.stringify(message));
    // Ignore outgoing messages.
    if (message.sender === 'background')
      return;

    var cmd = message.cmd;
    if (!cmd) {
      console.log('Message received with no command.');
      return;
    }

    var messageHandlerName = 'onMessage_' + cmd;
    var fn = global[messageHandlerName];
    if (typeof fn !== 'function') {
      console.log('Unknown message received: ' + cmd);
      return;
    }

    fn(message.data, sender);
  };

  port.onMessage.addListener(onMessage);
  port.onDisconnect.addListener(onDisconnect);
  postSettingsToPort(port, global.settings);
};


///
/// Message handlers
///
function onMessage_setSettings(data, sender) {
  setSettings(data);
}

function onMessage_toggleEnabled(data, sender) {
  toggleEnabled();
}

function onMessage_popupConnected(data, sender) {
  hideBadge();
}


///
/// Settings stuff
///
function loadSettings() {
  chrome.storage.sync.get('settings', function(data) {
    if (chrome.runtime.lastError) {
      console.log('Error loading settings: ' +
                  chrome.runtime.lastError.message);
      return;
    }

    if (!data.settings) {
      // If there is no data there yet, just use the defaults defined above.
      data.settings = global.settings;
    }

    // Force the settings to be updated.
    setSettings(data.settings, true, true);
  });
}

function saveSettings() {
  chrome.storage.sync.set({settings: global.settings}, function() {
    if (chrome.runtime.lastError) {
      console.log('Error saving settings: ' +
                  chrome.runtime.lastError.message);
      return;
    }
  });
}

function setSettings(newSettings, noSave, force) {
  var changed = false;
  var changedSettings = {};

  for (var key in newSettings) {
    var newValue = newSettings[key];
    var oldValue = global.settings[key];
    if (force || oldValue !== newValue) {
      changed = true;
      changedSettings[key] = newValue;
      global.settings[key] = newValue;
      var changedHandlerName = 'onSettingChanged_' + key;
      var fn = global[changedHandlerName];
      if (typeof fn === 'function')
        fn(newValue, oldValue);
    }
  }

  if (changed) {
    postSettingsToAllPorts(changedSettings);
    if (!noSave)
      saveSettings();
  }
}

function postSettingsToPort(port, changedSettings) {
  console.log('Sending settings to port: ' + port.name);
  port.postMessage({
    cmd: 'setSettings',
    data: changedSettings,
    sender: 'background'
  });
}

function postSettingsToAllPorts(changedSettings) {
  for (var i = 0; i < global.connectedPorts.length; ++i) {
    var port = global.connectedPorts[i];
    postSettingsToPort(port, changedSettings);
  }
}

function toggleEnabled() {
  setSettings({enabled: !global.settings.enabled});
  return global.settings.enabled;
}


///
/// SettingsChanged handlers
///
function onSettingChanged_enabled(enabled) {
  chrome.browserAction.setIcon(enabled ?
      global.enabledIcon :
      global.disabledIcon);
}


///
/// Handle rewriting web requests to include application/x-love-game mimetype.
///
chrome.webRequest.onHeadersReceived.addListener(
  function(info) {
    if (!global.settings.enabled) {
      console.log('skipping '+info.url+' because disabled.');
      return;
    }

    if (/dropbox\.com/.test(info.url)) {
      console.log('skipping '+info.url+' because it\'s a dropbox page.');
      return;
    }

    console.log(info.url);
    var newHeaders = [];
    for (var i = 0; i < info.responseHeaders.length; ++i) {
      var headerObj = info.responseHeaders[i];

      if (headerObj.name.toLowerCase() == 'content-disposition') {
        console.log('found content-disposition');
        if (/filename=.*?\.love/.test(headerObj.value)) {
          // Don't download!
          console.log('skipping header '+headerObj.name+'='+headerObj.value);
          continue;
        } else {
          // Content-Disposition, but no .love. Ignore.
          console.log('skipping '+info.url+' because it\'s not love.');
          return;
        }
      } else if (headerObj.name.toLowerCase() == 'content-type') {
        // Skip Content-Type
        console.log('skipping header '+headerObj.name+'='+headerObj.value);
        continue;
      }

      console.log('adding header '+headerObj.name+'='+headerObj.value);
      newHeaders.push(headerObj);
    }

    newHeaders.push({
      name: 'Content-Type',
      value: 'application/x-love-game'
    });

    return {
      responseHeaders: newHeaders
    };
  },
  { urls: ['*://*.love2d.org/*file.php?id=*',
           '*://*.dropboxusercontent.com/*.love?*',
           '*://*/*.love'] },
  ['blocking', 'responseHeaders']
);


///
/// Handle updating the badge (when updating or first installed).
///
function showUpdateBadge() {
  chrome.browserAction.setBadgeBackgroundColor({color: '#468847'});
  chrome.browserAction.setBadgeText({text: '\u2191'});
}

function hideBadge() {
  chrome.browserAction.setBadgeText({text: ''});
}

chrome.runtime.onInstalled.addListener(function(details) {
  if (details.reason === 'install') {
    // Show the test page on first install.
    chrome.tabs.create({url: 'test.html'});
  } else if (details.reason === 'update') {
    // Show a nice update badge (a vertical arrow) when the extension has
    // updated.
    showUpdateBadge();
  }
});


///
/// Add context menu to open any link as a .love file.
///
chrome.contextMenus.create({
  title: 'Open with LÖVELINESS',
  contexts: ['link'],
  onclick: function(info, tab) {
    chrome.tabs.create({
      url: 'context_menu.html?url='+encodeURIComponent(info.linkUrl)
    });
  }
});

loadSettings();
