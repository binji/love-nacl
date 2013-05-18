var enabled = true;

chrome.webRequest.onHeadersReceived.addListener(
  function (info) {
    if (!enabled) {
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

var enabledIcon = {'path': {'19': 'nyu19_on.png', '38': 'nyu38_on.png'}};
var disabledIcon = {'path': {'19': 'nyu19.png', '38': 'nyu38.png'}};

chrome.browserAction.setIcon(enabled ? enabledIcon : disabledIcon);
function toggleEnabled() {
  enabled = !enabled;
  console.log('toggleEnabled called! enabled=' + enabled);
  chrome.browserAction.setIcon(enabled ? enabledIcon : disabledIcon);
  return enabled;
}

function showUpdateBadge() {
  chrome.browserAction.setBadgeBackgroundColor({color: '#468847'});
  chrome.browserAction.setBadgeText({text: '\u2191'});
}

function hideBadge() {
  chrome.browserAction.setBadgeText({text: ''});
}

// Show the test page on first install.
chrome.runtime.onInstalled.addListener(function (details) {
  if (details.reason === 'install') {
    chrome.tabs.create({url: 'test.html'});
  } else if (details.reason === 'update') {
    showUpdateBadge();
  }
});

// Cool trick to modify auto-generated embed page:
// See https://groups.google.com/d/msg/native-client-discuss/UJu7VXvV_bw/pLc19D50gbwJ

function injectScript(tab) {
  // This is a minimal script that only checks to see if we should be
  // injecting more.
  chrome.tabs.executeScript(tab.id, {
    file: 'inject.js',
    runAt: 'document_start'
  });
}

chrome.tabs.onCreated.addListener(function (tab) {
  injectScript(tab);
});
chrome.tabs.onUpdated.addListener(function (tabId, changeInfo, tab) {
  injectScript(tab);
});

chrome.runtime.onMessage.addListener(function (message, sender, sendResponse) {
  if (message === 'injected') {
    // If the tab sent us a response, it must be a NaCl mimetype handler page.
    // Inject the full script to resize the embed, etc...
    chrome.tabs.executeScript(sender.tab.id, {
      file: 'injected.js',
      runAt: 'document_start'
    });
  }
});

// Add context menu, too.
chrome.contextMenus.create({
  title: 'Open with LÖVELINESS',
  contexts: ['link'],
  onclick: function (info, tab) {
    chrome.tabs.create({url: 'context_menu.html'}, function (tab) {
      chrome.tabs.sendMessage(tab.id, info.linkUrl);
    });
  }
});
