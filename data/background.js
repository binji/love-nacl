console.log('adding listener!');

var enabled = false;

chrome.webRequest.onHeadersReceived.addListener(
  function (info) {
    if (!enabled) {
      console.log('skipping '+info.url+' because disabled.');
      return;
    }

    console.log(info.url);
    var newHeaders = [];
    for (var i = 0; i < info.responseHeaders.length; ++i) {
      var headerObj = info.responseHeaders[i];
      console.log(headerObj);
      if (headerObj.name != 'Content-Disposition' ||
          !/filename=.*?\.love\s*$/.test(headerObj.value)) {
        newHeaders.push(headerObj);
      }
    }

    return {
      responseHeaders: newHeaders
    };
  },
  { urls: ['*://*.love2d.org/*file.php?id=*'] },
  ['blocking', 'responseHeaders']
);

chrome.webRequest.onHeadersReceived.addListener(
  function (info) {
    if (!enabled) {
      console.log('skipping '+info.url+' because disabled.');
      return;
    }

    console.log(info.url);
    var newHeaders = [];
    for (var i = 0; i < info.responseHeaders.length; ++i) {
      var headerObj = info.responseHeaders[i];
      console.log(headerObj);
      if (headerObj.name != 'Content-Type')
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
  { urls: ['*://*/*.love', '*://*/*.exe'] },
  ['blocking', 'responseHeaders']
);

var enabledIcon = {'path': {'19': 'nyu19_on.png', '38': 'nyu38_on.png'}};
var disabledIcon = {'path': {'19': 'nyu19.png', '38': 'nyu38.png'}};

chrome.browserAction.onClicked.addListener(
  function (tab) {
    enabled = !enabled;
    console.log('browserAction clicked! enabled=' + enabled);
    chrome.browserAction.setIcon(enabled ? enabledIcon : disabledIcon);
  }
);
