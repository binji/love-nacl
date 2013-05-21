(function () {

var fileSystemSize = 5 * 1024 * 1024;  // 5M should be enough for anybody.
var messageMap = {
  // HACK(binji):  these messages aren't filesystem related. Probably should
  // rename this file...
  'back': onMessageBack,
  'close': onMessageClose,
  'reload': onMessageReload,

  'getFiles': onMessageGetFiles,
  'queryFileSystem': onMessageQueryFilesystem,
  'requestFileSystem': onMessageRequestFileSystem,
}

function onMessageBack(data, response) {
  window.history.back();
}

function onMessageClose(data, response) {
  window.close();
}

function onMessageReload(data, response) {
  window.location.reload();
}

function onMessageGetFiles(data, response) {
  var outstandingAsyncCalls = 0;

  function onAsyncCallFinished() {
    if (--outstandingAsyncCalls === 0) {
      response({done: true});
    }
  }

  function readDirectory(dirEntry, depth) {
    var dirReader = dirEntry.createReader();

    function readEntries() {
      outstandingAsyncCalls++;

      function onReadEntriesError(err) {
        onAsyncCallFinished();
      }

      function onReadEntries(results) {
        if (!results.length) {
          // Done with this directory.
          onAsyncCallFinished();
          return;
        }

        for (var i = 0; i < results.length; ++i) {
          var entry = results[i];

          if (entry.isFile) {
            response({type: 'file', path: entry.fullPath}, false);
          } else if (entry.isDirectory) {
            response({type: 'dir', path: entry.fullPath}, false);
            readDirectory(entry, depth + 1);
          }
        }

        readEntries();
        onAsyncCallFinished();
      }

      dirReader.readEntries(onReadEntries, onReadEntriesError);
    }

    readEntries();
  }

  function onRequestSuccess(fileSystem) {
    readDirectory(fileSystem.root, 0);
  }

  function onRequestError(err) {
    response({done: true});
  }

  window.webkitRequestFileSystem(window.PERSISTENT, fileSystemSize,
      onRequestSuccess, onRequestError);
}

function onMessageQueryFilesystem(data, response) {
  function onSuccess(bytesUsed, bytesQuota) {
    console.log('usage: '+bytesUsed+' quota: '+bytesQuota);
    if (bytesQuota > 0) {
      response({ok: true, bytesUsed: bytesUsed, bytesQuota: bytesQuota});
    } else {
      response({ok: false});
    }
  }

  function onError(err) {
    response({ok: false});
  }

  navigator.webkitPersistentStorage.queryUsageAndQuota(onSuccess, onError);
}

function onMessageRequestFileSystem(data, response) {
  function onSuccess(fileSystem) {
    response({ok: true});
  }

  function onError(err) {
    response({ok: false});
  }

  navigator.webkitPersistentStorage.requestQuota(
      fileSystemSize, onSuccess, onError);
}

function onWindowMessage(e) {
  if (e.source !== window)
    return;

  var cmd = e.data.cmd;
  if (cmd === 'response')
    return;

  var func = messageMap[cmd];
  if (func) {
    var responseFunc = function (responseData, finished) {
      var data = {
        cmd: 'response',
        id: e.data.id,
        data: responseData,
        finished: finished !== undefined ? finished : true,
      };
      window.postMessage(data, '*');
    };
    func(e.data, responseFunc);
  } else {
    console.log('Unknown window command: '+cmd);
  }
}

window.addEventListener('message', onWindowMessage);

})();
