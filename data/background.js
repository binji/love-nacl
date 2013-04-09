console.log('adding listener!');

chrome.webRequest.onHeadersReceived.addListener(
  function (info) {
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
  { urls: ['*://*/*.love'] },
  ['blocking', 'responseHeaders']
);
