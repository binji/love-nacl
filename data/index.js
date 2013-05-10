function $(selector) {
  return document.querySelector(selector);
}

function onDragOver(e) {
  e.dataTransfer.dropEffect = 'copy';
  $('#dropTarget').classList.add('dragover');
  e.stopPropagation();
  e.preventDefault();
}

function onDragLeave(e) {
  $('#dropTarget').classList.remove('dragover');
  e.stopPropagation();
  e.preventDefault();
}

function onDrop(e) {
  var file = e.dataTransfer.files[0];
  var url = URL.createObjectURL(file);
  document.body.removeChild($('#dropTarget'));

  function onLoad(e) {
  }

  function onMessage(e) {
    if (typeof e.data !== 'string')
      return;
    var msg = e.data;
    console.log('got message: ' + msg);
    var setWindowMsg = 'setWindow:';
    if (msg === 'OK') {
      // loaded successfully.
      URL.revokeObjectURL(url);
    } else if (msg.lastIndexOf(setWindowMsg, 0) === 0) {
      // TODO(binji): this is copied from inject.js -- would be nicer to be
      // able to share this code.
      var value = msg.substr(setWindowMsg.length);
      var comma = value.indexOf(',');
      if (comma >= 0) {
        var embedEl = $('embed');
        var w = parseInt(value.substr(0, comma), 10);
        var h = parseInt(value.substr(comma + 1), 10);
        embedEl.setAttribute('width', w + 'px');
        embedEl.setAttribute('height', h + 'px');
      }
    }
  }

  function createModule() {
    var listenerEl = document.createElement('div');
    listenerEl.setAttribute('id', 'listener');
    listenerEl.classList.add('fullscreen');
    listenerEl.addEventListener('load', onLoad, true);
    listenerEl.addEventListener('message', onMessage, true);

    // Get the manifest to find the correct nmf to use.
    var manifest = chrome.runtime.getManifest();
    var nmf = manifest.nacl_modules[0].path;

    var embedEl = document.createElement('embed');
    embedEl.setAttribute('src', nmf);
    embedEl.setAttribute('type', 'application/x-nacl');
    embedEl.setAttribute('love_src', url);
    embedEl.setAttribute('width', '100%');
    embedEl.setAttribute('height', '100%');

    listenerEl.appendChild(embedEl);
    document.body.appendChild(listenerEl);
  }

  function onTransitionEnd(e) {
    document.body.removeEventListener('transitionend', onTransitionEnd, true);
    createModule();
  }

  document.body.addEventListener('transitionend', onTransitionEnd, true);
  document.body.classList.add('loading');

  e.stopPropagation();
  e.preventDefault();
}

document.addEventListener('DOMContentLoaded', function() {
  $('#dropTarget').addEventListener('dragleave', onDragLeave, false);
  $('#dropTarget').addEventListener('dragover', onDragOver, false);
  $('#dropTarget').addEventListener('drop', onDrop, false);
});
