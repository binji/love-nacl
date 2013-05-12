function $(selector) {
  return document.querySelector(selector);
}

function onMessage(e) {
  if (typeof e.data !== 'string')
    return;
  var msg = e.data;
  var setWindowMsg = 'setWindow:';
  if (msg === 'BYE') {
    window.close();
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

      // Resize the app window as well.
      chrome.app.window.current().resizeTo(w, h);
    }
  }
}

function onCrash(e) {
  window.close();
}

document.addEventListener('DOMContentLoaded', function() {
  var listenerEl = $('#listener');
  listenerEl.addEventListener('message', onMessage, true);
  listenerEl.addEventListener('crash', onCrash, true);

  var embedEl = document.createElement('embed');
  embedEl.setAttribute('src', 'love_release.nmf');
  embedEl.setAttribute('type', 'application/x-nacl');
  embedEl.setAttribute('love_src', '{{love_src}}');
  embedEl.setAttribute('width', '100%');
  embedEl.setAttribute('height', '100%');

  listenerEl.appendChild(embedEl);
});
