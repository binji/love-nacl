console.log('adding listener!');

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
chrome.browserAction.onClicked.addListener(
  function (tab) {
    enabled = !enabled;
    console.log('browserAction clicked! enabled=' + enabled);
    chrome.browserAction.setIcon(enabled ? enabledIcon : disabledIcon);
  }
);
